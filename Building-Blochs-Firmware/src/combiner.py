import os
import sys
import re

# Regular expressions to match include directives
LOCAL_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"')
SYSTEM_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*<([^>]+)>')

# Search paths and file extensions
SEARCH_PATHS = ['.', '../include']
HEADER_EXTS = {'.h', '.hpp', '.hxx', '.hh'}
SOURCE_EXTS = {'.cpp', '.cc', '.cxx', '.c'}

class Amalgamator:
    def __init__(self):
        self.processed_files = set()
        self.system_includes = set()
        self.output_lines = []

    def find_file(self, filename):
        """Looks for the file in the defined search paths."""
        for path in SEARCH_PATHS:
            filepath = os.path.join(path, filename)
            if os.path.isfile(filepath):
                return os.path.normpath(filepath)
        return None

    def process_file(self, filepath):
        """Recursively reads files and resolves #include directives."""
        abs_path = os.path.abspath(filepath)
        
        # Act as a universal #pragma once / include guard
        if abs_path in self.processed_files:
            return  
        
        self.processed_files.add(abs_path)

        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                lines = f.readlines()
        except Exception as e:
            sys.stderr.write(f"// [WARNING] Could not read {filepath}: {e}\n")
            return

        self.output_lines.append(f"// --- Begin {filepath} ---\n")
        
        for line in lines:
            # Strip out #pragma once to avoid compiler warnings in the single file
            if line.strip() == "#pragma once":
                continue

            local_match = LOCAL_INCLUDE_RE.match(line)
            sys_match = SYSTEM_INCLUDE_RE.match(line)

            if local_match:
                included_file = local_match.group(1)
                found_path = self.find_file(included_file)
                if found_path:
                    # Recursively process the found local header
                    self.process_file(found_path)
                else:
                    # File not found in our paths (e.g., external library), keep the include
                    self.output_lines.append(line)
            elif sys_match:
                # Extract system includes (<...>) and hoist them to the top of the final file
                self.system_includes.add(line.strip())
            else:
                self.output_lines.append(line)
                
        self.output_lines.append(f"// --- End {filepath} ---\n\n")

    def generate(self):
        """Finds all files, processes them, and writes to stdout."""
        files_in_dir = [f for f in os.listdir('.') if os.path.isfile(f)]
        headers = []
        sources = []
        
        # Categorize files in the current directory
        for f in files_in_dir:
            _, ext = os.path.splitext(f)
            if ext in HEADER_EXTS:
                headers.append(f)
            elif ext in SOURCE_EXTS:
                sources.append(f)
                
        # Sort alphabetically to ensure deterministic output across different runs
        headers.sort()
        sources.sort()
        
        # Process headers first, then sources. 
        # This ensures standalone structs/classes are declared before implementation
        # if they weren't explicitly included in a source file.
        for h in headers:
            self.process_file(h)
        for s in sources:
            self.process_file(s)

        # Write output to stdout
        sys.stdout.write("#define LV_CONF_SKIP 1\n// --- Amalgamated System Includes ---\n")
        for sys_inc in sorted(self.system_includes):
            sys.stdout.write(f"{sys_inc}\n")
        sys.stdout.write("\n")
        
        for line in self.output_lines:
            sys.stdout.write(line)

if __name__ == "__main__":
    Amalgamator().generate()