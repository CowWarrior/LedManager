### Generated using https://chatgpt.com/c/678ef0d4-fe64-800e-979c-5da363d36f07
# Note: To install jtmlmin and jsmin libraries use the following commands (In PowerShell terminal of VSCode ):
#           pip install htmlmin jsmin
#
# Also modify the platformio.ini to include the following line:
#           extra_scripts = pre:auto_minify_files.py
#
#Note: My build failed and was not able to locate the installed modules. I had to install the modules in the global environemnt
#   1. Open a Command Prompt (or PowerShell)
#   2. cd C:\Users\pplap\.platformio\penv\Scripts>
#   3. .\pip.exe install htmlmin jsmin
#   IMPORTANT! the .\pip.exe is important otherwise it will use whatever virtual environment is active at the moment.
#
# After reading documentation, adding a try/catch block to install libraries should be working without manual install.
#   https://docs.platformio.org/en/latest/scripting/examples/extra_python_packages.html
import os
import time

Import("env")

#html minifier
try:
    from htmlmin import minify as minify_html
except ImportError:
    env.Execute("$PYTHONEXE -m pip install htmlmin")

#javascript minifier
try:
    from jsmin import jsmin
except ImportError:
    env.Execute("$PYTHONEXE -m pip install jsmin")

# Define directories
source_dir = ".\data-source"
dest_dir = ".\data\www"

# Track modification times
timestamp_file = ".file_timestamps"

def get_file_timestamps():
    """Load the last modified times of files."""
    if not os.path.exists(timestamp_file):
        return {}
    with open(timestamp_file, "r") as f:
        return {line.split(",")[0]: float(line.split(",")[1]) for line in f}

def save_file_timestamps(timestamps):
    """Save the last modified times of files."""
    with open(timestamp_file, "w") as f:
        for filepath, mtime in timestamps.items():
            f.write(f"{filepath},{mtime}\n")

def minify_file(source_path, dest_path):
    with open(source_path, 'r', encoding='utf-8') as src:
        content = src.read()
        if (source_path.endswith(".htm") or source_path.endswith(".html")) :
            minified = minify_html(content, remove_empty_space=True)
        elif source_path.endswith(".js"):
            minified = jsmin(content)
        else:
            minified = content

    with open(dest_path, 'w', encoding='utf-8') as dest:
        dest.write(minified)
    print(f"Minified {source_path} -> {dest_path}")

def copy_and_minify():
    print("Minifiying Javascript and HTML files...")
    timestamps = get_file_timestamps()
    new_timestamps = {}
    changes_detected = False

    for root, _, files in os.walk(source_dir):
        for file in files:
            if file.endswith((".html", ".htm", ".js")):
                source_path = os.path.join(root, file)
                relative_path = os.path.relpath(source_path, source_dir)
                dest_path = os.path.join(dest_dir, relative_path)

                # Check if the file has changed
                mtime = os.path.getmtime(source_path)
                new_timestamps[source_path] = mtime
                if source_path not in timestamps or mtime > timestamps[source_path]:
                    # File has been modified
                    os.makedirs(os.path.dirname(dest_path), exist_ok=True)
                    minify_file(source_path, dest_path)
                    changes_detected = True
                else:
                    print(f"No changes in {source_path}, skipped.")

    # Save new timestamps
    save_file_timestamps(new_timestamps)

    if changes_detected:
        print("Files updated and minified.")
    else:
        print("No changes detected.")

#execute the main minication actions...
copy_and_minify()
