#!/usr/bin/env python3
"""Draw a declaration-level UML class diagram of a C++ project.

Grain: one box per class/struct/enum — the same grain as IntelliJ's "Show
Diagram", not the instruction-level Graphviz dumps LLVM produces. Input is
Clang's JSON AST, so the boxes and arrows come from the compiler's own model
of the code, not from regexes over text.

    tools/class_diagram.py include/zcc -Iinclude
    tools/class_diagram.py include/zcc -Iinclude -o build/uml --svg
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import tempfile
import sys
from dataclasses import dataclass, field
from pathlib import Path

ACCESS_KINDS = {"AccessSpecDecl"}
METHOD_KINDS = {"CXXMethodDecl", "CXXConstructorDecl", "CXXDestructorDecl",
                "CXXConversionDecl"}
RECORD_KINDS = {"CXXRecordDecl", "ClassTemplateDecl"}
WANT = {"CXXRecordDecl", "EnumDecl"}


@dataclass
class Type:
    name: str
    kind: str            # class | struct | enum
    fields: list[tuple[str, str, str]] = field(default_factory=list)
    methods: list[tuple[str, str, str]] = field(default_factory=list)
    enumerators: list[str] = field(default_factory=list)
    bases: list[str] = field(default_factory=list)


def run_ast(clang: str, headers: list[Path], extra: list[str], keep: str | None) -> list[dict]:
    """One translation unit that includes every header, so Clang emits one AST.

    Without the dump filter the JSON is hundreds of megabytes of libc++
    declarations; the filter keeps only the project's own namespace."""
    with tempfile.NamedTemporaryFile("w", suffix=".cpp", delete=False) as unit:
        unit.write("".join(f'#include "{h.resolve()}"\n' for h in headers))
        unit_path = unit.name

    cmd = [clang, *extra, "-fsyntax-only", "-Xclang", "-ast-dump=json"]
    if keep:
        cmd += ["-Xclang", f"-ast-dump-filter={keep}"]
    cmd.append(unit_path)

    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    Path(unit_path).unlink(missing_ok=True)
    if proc.returncode != 0:
        sys.exit(f"clang failed:\n{proc.stderr.strip()[:2000]}")

    # -ast-dump-filter prints one JSON document per surviving top-level decl.
    decoder = json.JSONDecoder()
    documents, index, text = [], 0, proc.stdout
    while index < len(text):
        while index < len(text) and text[index].isspace():
            index += 1
        if index >= len(text):
            break
        document, index = decoder.raw_decode(text, index)
        documents.append(document)
    if not documents:
        sys.exit("clang produced no AST — is the filter namespace right?")
    return documents


def short_type(qual: str) -> str:
    qual = re.sub(r"\bstd::(?:__1::)?", "", qual)
    qual = qual.replace("std::", "")
    return re.sub(r"\s+", " ", qual).strip()


def collect(node: dict, namespace: list[str], out: dict[str, Type]) -> None:
    kind = node.get("kind")

    if kind == "NamespaceDecl":
        name = node.get("name") or ""
        for child in node.get("inner", []):
            collect(child, namespace + ([name] if name else []), out)
        return

    if kind in WANT and not node.get("isImplicit"):
        name = node.get("name") or ""
        complete = node.get("completeDefinition") or kind == "EnumDecl"
        if name and complete:
            qualified = "::".join(namespace + [name])
            entry = out.setdefault(
                qualified,
                Type(qualified, node.get("tagUsed", "class")),
            )
            if kind == "EnumDecl":
                entry.kind = "enum"
                entry.enumerators = [
                    c["name"] for c in node.get("inner", [])
                    if c.get("kind") == "EnumConstantDecl"
                ]
            else:
                entry.bases = [
                    short_type(b["type"]["qualType"]) for b in node.get("bases", [])
                ]
                read_members(node, entry, default_access="public" if entry.kind == "struct" else "private")
            return

    for child in node.get("inner", []):
        collect(child, namespace, out)


def read_members(record: dict, entry: Type, default_access: str) -> None:
    access = default_access
    for child in record.get("inner", []):
        kind = child.get("kind")
        if kind in ACCESS_KINDS:
            access = child.get("access", access)
            continue
        if child.get("isImplicit"):
            continue
        if kind == "FieldDecl" and child.get("name"):
            entry.fields.append((access, short_type(child["type"]["qualType"]),
                                 child["name"]))
        elif kind in METHOD_KINDS and child.get("name"):
            signature = short_type(child["type"]["qualType"])
            entry.methods.append((access, signature, child["name"]))


def relations(types: dict[str, Type]) -> list[tuple[str, str, str]]:
    """(from, to, kind) where kind is inherit | compose | associate | depend."""
    names = {t.name.split("::")[-1]: t.name for t in types.values()}
    edges: dict[tuple[str, str, str], None] = {}

    def mentioned(text: str) -> list[str]:
        hits = []
        for short, full in names.items():
            if re.search(rf"\b{re.escape(short)}\b", text):
                hits.append(full)
        return hits

    for entry in types.values():
        for base in entry.bases:
            if base in types:
                edges[(entry.name, base, "inherit")] = None
        for _, field_type, _ in entry.fields:
            for target in mentioned(field_type):
                if target == entry.name:
                    continue
                kind = "associate" if ("&" in field_type or "*" in field_type) else "compose"
                edges[(entry.name, target, kind)] = None
        for _, signature, _ in entry.methods:
            for target in mentioned(signature):
                if target == entry.name:
                    continue
                edges[(entry.name, target, "depend")] = None
    rank = {"inherit": 4, "compose": 3, "associate": 2, "depend": 1}
    best: dict[tuple[str, str], str] = {}
    for source, target, kind in edges:
        key = (source, target)
        if key not in best or rank[kind] > rank[best[key]]:
            best[key] = kind
    return sorted((a, b, k) for (a, b), k in best.items())


def html(text: str) -> str:
    """Graphviz HTML-like labels take entities, not backslash escapes."""
    return (text.replace("&", "&amp;").replace("<", "&lt;")
                .replace(">", "&gt;").replace('"', "&quot;"))


def signature(sig: str, name: str) -> str:
    """`void (const SourceFile &, DiagnosticEngine &)` -> `Lexer(const
    SourceFile &, DiagnosticEngine &)` / `Token ()` -> `next() : Token`."""
    if "(" not in sig:
        return name
    ret = sig[: sig.find("(")].strip()
    params = sig[sig.find("(") + 1 : sig.rfind(")")].strip()
    shown = f"{name}({params})"
    return shown if ret in ("", "void") else f"{shown} : {ret}"


def to_dot(types: dict[str, Type]) -> str:
    out = [
        "digraph uml {",
        "  rankdir=BT;",
        '  graph [fontname="Helvetica", fontsize=11, labelloc="t", label="zcc — class diagram"];',
        '  node [shape=plaintext, fontname="Helvetica", fontsize=10];',
        '  edge [fontname="Helvetica", fontsize=8, color="#555555"];',
    ]

    for entry in sorted(types.values(), key=lambda t: t.name):
        node = re.sub(r"[^A-Za-z0-9_]", "_", entry.name)
        if entry.kind == "enum":
            rows = "<i>«enumeration»</i><br/>" + "<br/>".join(
                html(e) for e in entry.enumerators[:12]
            )
            if len(entry.enumerators) > 12:
                rows += f"<br/>… {len(entry.enumerators) - 12} more"
            label = f"<<table border='1' cellborder='0' cellspacing='0'><tr><td bgcolor='#eef3fb'><b>{html(entry.name.split('::')[-1])}</b></td></tr><tr><td>{rows}</td></tr></table>>"
            out.append(f"  {node} [label={label}];")
            continue

        name_cell = html(entry.name.split("::")[-1])
        stereotype = "<i>«struct»</i><br/>" if entry.kind == "struct" else ""
        header = f"<tr><td bgcolor='#dce8f7'>{stereotype}<b>{name_cell}</b></td></tr>"
        body = ""
        if entry.fields:
            body += "<tr><td align='left'>" + "<br/>".join(
                f"{'-' if a == 'private' else '+'} {html(n)} : {html(t)}"
                for a, t, n in entry.fields
            ) + "</td></tr>"
        if entry.methods:
            body += "<tr><td align='left'>" + "<br/>".join(
                f"{'-' if a == 'private' else '+'} {html(signature(sig, n))}"
                for a, sig, n in entry.methods
            ) + "</td></tr>"
        label = f"<<table border='1' cellborder='1' cellspacing='0' cellpadding='4'>{header}{body}</table>>"
        out.append(f"  {node} [label={label}];")

    styles = {
        "inherit": ' [arrowhead=onormal, style=solid]',
        "compose": ' [arrowhead=diamond, style=solid]',
        "associate": ' [arrowhead=vee, style=solid]',
        "depend": ' [arrowhead=vee, style=dashed]',
    }
    for source, target, kind in relations(types):
        a = re.sub(r"[^A-Za-z0-9_]", "_", source)
        b = re.sub(r"[^A-Za-z0-9_]", "_", target)
        out.append(f"  {a} -> {b}{styles[kind]};")

    out.append("}")
    return "\n".join(out) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="+", type=Path, help="headers or directories")
    parser.add_argument("-o", "--outdir", type=Path, default=Path("build/uml"))
    parser.add_argument("--clang", default="clang++")
    parser.add_argument("--svg", action="store_true", help="also render .svg")
    parser.add_argument("--std", default="c++17")
    parser.add_argument("--filter", help="namespace to keep (default: guessed)")
    args, extra = parser.parse_known_args()

    headers: list[Path] = []
    for path in args.paths:
        if path.is_dir():
            headers.extend(sorted(path.rglob("*.h")) + sorted(path.rglob("*.hpp")))
        else:
            headers.append(path)
    if not headers:
        sys.exit("no headers found")
    if not any(a.startswith("-std=") for a in extra):
        extra = [f"-std={args.std}"] + extra

    keep = args.filter
    if keep is None:
        found = [m for h in headers for m in re.findall(r"^namespace\s+(\w+)", h.read_text(), re.M)]
        keep = max(set(found), key=found.count) if found else None

    types: dict[str, Type] = {}
    for document in run_ast(args.clang, headers, extra, keep):
        for child in document.get("inner", []):
            collect(child, [], types)
    if not types:
        sys.exit("no types found — check the include path")

    args.outdir.mkdir(parents=True, exist_ok=True)
    dot_path = args.outdir / "classes.dot"
    dot_path.write_text(to_dot(types))
    if args.svg and (dot := shutil.which("dot")):
        subprocess.run([dot, "-Tsvg", str(dot_path), "-o", str(dot_path.with_suffix(".svg"))],
                       check=True)
    print(f"{len(types)} types, {len(relations(types))} relations -> {dot_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
