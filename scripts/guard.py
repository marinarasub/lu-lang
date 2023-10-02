#ifndef LU_GUARD_H
#define LU_GUARD_H

from sys import argv
import os.path as path
import os
import argparse

def macro_name(project: str, name: str) -> str:
    return f"{project.upper()}_{name.upper()}_H"

def gen_file(args, fname):
    if (not fname):
        raise RuntimeError("No file given!")
    if (not args.project):
        raise RuntimeError("Project name must be defined!")
    name = args.name
    if (not name):
        name = path.splitext(path.basename(fname))[0]
    macro = macro_name(args.project, name)
    with open(fname, 'r+') as file:
        contents = file.read()
        if args.auto:
            # Check for #define
            split = contents.split(maxsplit=2)
            first = split[0] if len(split) > 0 else ""
            second = split[1] if len(split) > 1 else ""
            if first == "#ifndef" or (first == "#" and second == "ifndef"):
                print(f"Detected guard in {fname}, skipping")
                return
        file.truncate(0)
        file.seek(0)
        print(f"#ifndef {macro}", file=file)
        print(f"#define {macro}", file=file)
        print("", file=file)
        print(contents, file=file)
        print("", file=file)
        print(f"#endif // {macro}", file=file)
        print(f"Added guard to {fname}")
        

def gen_console(args):
    if (not args.project or not args.name):
        raise RuntimeError("Project and specific name must be defined!")
    macro = macro_name(args.project, args.name)
    print(f"#ifndef {macro}")
    print(f"#define {macro}")
    print()
    print(f"#endif // {macro}")
    return

def main(argv: "list[str]") -> int:
    parser = argparse.ArgumentParser(description="Generate preprocessor header guard")
    parser.add_argument("-i", "--input", help="file name to apply to (if none, output text)")
    parser.add_argument("-p", "--project", help="project name in form {project name}_{name}_H", default="LU")
    parser.add_argument("-n", "--name", help="specific name in form {project name}_{name}_H")
    parser.add_argument("-d", "--dir", help="add for all folders in directory (not compatible with custom name)")
    parser.add_argument("-a", "--auto", help="Only add guard if first line is not #define", default=True)
    args = parser.parse_args(argv[1:])

    if args.dir:
        for (root, dirs, files) in os.walk(args.dir):
            for fname in files:
                ext = os.path.splitext(fname)[1]
                if ext == ".h" or ext == ".hpp":
                    gen_file(args, os.path.join(root, fname))
    elif args.input:
        gen_file(args, args.input)
    else:
        gen_console(args)

if __name__ == '__main__':
    exit(main(argv))

#endif // LU_GUARD_H
