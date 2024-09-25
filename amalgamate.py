# Simple script to amalgamate C source files into a single file.
# Don't use this in other projects, it's just a simple script specific to this project.

import os
import re
import argparse
import networkx as nx

logging = False


def read_file(filepath):
    """Read the content of a file."""
    with open(filepath, "r") as file:
        return file.read()


def write_file(filepath, content):
    """Write content to a file."""
    with open(filepath, "w") as file:
        file.write(content)


def log(message):
    """Log a message to the amalgmate.log file."""
    if not logging:
        return
    with open("amalgamate.log", "a") as log_file:
        log_file.write(message + "\n")


def extract_includes(content):
    """Extract #include directives from content."""
    includes = set()
    include_pattern = re.compile(r'#include\s*"(.*?)"')
    for match in include_pattern.finditer(content):
        includes.add(match.group(1))
    return includes


def build_include_graph(dir, includes):
    """Build a directed graph from the include directives."""
    graph = nx.DiGraph()
    for include in includes:
        dependencies = extract_includes(read_file(dir + "/" + include))
        log(f"Dependencies for {include}: {dependencies}")
        for dependency in dependencies:
            graph.add_edge(include, dependency)
    return graph


def get_ordered_includes(dir, includes):
    """Order the includes based on the dependencies."""
    graph = build_include_graph(dir, includes)
    return list(reversed(list(nx.topological_sort(graph))))


def find_local_headers(source_files):
    """Collect all local headers used in the source files."""
    source_includes = set()

    # We assume that all files are in the same directory
    dir = os.path.dirname(source_files[0])
    for filepath in source_files:
        content = read_file(filepath)
        includes = extract_includes(content)
        source_includes.update(includes)
    all_includes = source_includes.copy()
    for include in source_includes:
        content = read_file(dir + "/" + include)
        includes = extract_includes(content)
        all_includes.update(includes)
    log("Finding ordered includes in " + dir)
    ordered = get_ordered_includes(dir, all_includes)
    log("Ordered includes: " + str(ordered))

    # Some headers are only included by source files
    # and not by other headers. We need to manually add them
    # to the ordered list
    for include in all_includes:
        if include not in ordered:
            ordered.append(include)
    return ordered


def remove_header_guards(content):
    """Remove header guards from content."""
    lines = content.splitlines()
    new_lines = []
    state = "find_first_ifndef"
    guard_name = None
    depth = 0
    for line in lines:
        if state == "find_first_ifndef":
            # Ignore all leading lines until we find the first #ifndef directive
            if not line.startswith("#ifndef"):
                new_lines.append(line)
                continue
            match = re.match(r"#ifndef\s+(.*)[//.*]{0,1}", line)
            if match:
                guard_name = match.group(1)
                state = "find_first_define"
            else:
                raise ValueError("Expected header guard #ifndef directive")
        elif state == "find_first_define":
            # We expect the next line to be the #define directive
            match = re.match(r"#define\s+(.*)[//.*]{0,1}", line)
            if match and match.group(1) == guard_name:
                state = "find_endif"
            else:
                raise ValueError("Expected header guard #define directive")
        elif state == "find_endif":
            # Count the number of nested #ifndef, #ifdef and #if directives
            # If we're at the same level as the first #ifndef directive
            # then we've found the end of the header guard
            if (
                line.startswith("#ifndef")
                or line.startswith("#ifdef")
                or line.startswith("#if")
            ):
                depth += 1
                new_lines.append(line)
            elif line.startswith("#endif"):
                if depth == 0:
                    state = "done"
                else:
                    depth -= 1
                    new_lines.append(line)
            else:
                new_lines.append(line)
        elif state == "done":
            new_lines.append(line)
    return "\n".join(new_lines)


def remove_local_includes(content):
    """Remove obsolete #include directives from content."""
    lines = content.splitlines()
    new_lines = []

    for line in lines:
        if line.startswith("#include") and re.search(r'#include\s*"(.*?)"', line):
            continue
        elif line.startswith("#include") and re.search(r"#include\s*<th.h>", line):
            continue
        else:
            new_lines.append(line)

    return "\n".join(new_lines)

def remove_unneeded_lines(content):
    """Remove line macros and command-line comments from content."""
    lines = content.splitlines()
    new_lines = []

    for line in lines:
        if line.startswith("#line"):
            continue
        elif line.startswith("/* Command-line: "):
            continue
        else:
            new_lines.append(line)

    return "\n".join(new_lines)

def amalgamate(source_files, output_file):
    """Amalgamate C project files into a single file."""

    amalgamated_content = ""
    # Top level includes and defines
    amalgamated_content += "#define TH_WITH_AMALGAMATION 1\n"
    amalgamated_content += '#include "th.h"\n'

    # Find local headers used in the source files
    dir = os.path.dirname(source_files[0])
    local_headers = find_local_headers(source_files)

    # Remove local #include directives from the local headers
    # and write them to the output file
    for header in local_headers:
        log(f"Adding {header} to amalgamated file")
        content = remove_local_includes(read_file(dir + "/" + header))
        content = remove_header_guards(content)
        amalgamated_content += f"/* Start of {header} */\n"
        amalgamated_content += content + "\n"
        amalgamated_content += f"/* End of {header} */\n"

    # Add content of each file
    for filepath in source_files:
        content = remove_local_includes(read_file(filepath))
        content = remove_unneeded_lines(content)
        amalgamated_content += f"/* Start of {filepath} */\n"
        amalgamated_content += content + "\n"
        amalgamated_content += f"/* End of {filepath} */\n"

    # Write the final amalgamated content to the output file
    write_file(output_file, amalgamated_content)
    print(f"Amalgamated file created at {output_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Amalgamate C source files into a single file."
    )
    parser.add_argument(
        "source_files", nargs="+", help="The source files to amalgamate."
    )
    parser.add_argument(
        "-o",
        "--output",
        default="amalgamated.c",
        help="The output file for the amalgamated code.",
    )

    args = parser.parse_args()
    amalgamate(args.source_files, args.output)
