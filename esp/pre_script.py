Import("env")
import os
import gzip
import datetime
import hashlib

# Install missing package
try:
    import htmlmin
except ImportError:
    env.Execute("$PYTHONEXE -m pip install htmlmin")
    import htmlmin

# Path to the HTML file you want to compress
html_input_dir = "data"


def str_replace(s, search, replacement):
    return s.replace(search, replacement)


def write_compressed_data_to_header(filename, compressed_data, hdr_path):
    var_name = filename.replace(".", "_")
    # Write the name of the compressed data variable
    with open(hdr_path, "a") as f:
        f.write("const uint8_t {}[] PROGMEM = {{\n".format(var_name))

        # Write the compressed data as a byte array
        for i, byte in enumerate(compressed_data):
            if i != 0:
                if i % 16 == 0:
                    f.write(",\n")
                else:
                    f.write(", ")
            f.write("0x{:02x}".format(byte))
        f.write("\n};\n")

        # Write the constant with the size of the compressed data
        f.write("const size_t {}_size = {};\n\n".format(var_name, len(compressed_data)))


def write_string_data_to_header(filename, data, hdr_path):
    var_name = filename.replace(".", "_")
    # Generate the C header file containing the HTML data as a char array
    with open(hdr_path, "a", encoding="utf-8") as f:
        compressed_code = htmlmin.minify(data, remove_comments=True)
        f.write('const char {}[] PROGMEM = R"=====(\n'.format(var_name))
        f.write(compressed_code)
        f.write(')=====";\n\n')


def write_to_file(output_dir, filename, input_str):
    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, filename)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(input_str)


def compute_html_hash(device_name):
    hasher = hashlib.sha256()
    for filename in sorted(os.listdir(html_input_dir)):
        if filename.endswith(".html"):
            path = os.path.join(html_input_dir, filename)
            with open(path, "rb") as f:
                hasher.update(f.read())
    hasher.update(device_name.encode("utf-8"))
    return hasher.hexdigest()


def generate_html_pages(target, source, env):
    print("PRE SCRIPT: Generating web related files and variables...")
    build_flags = env.ParseFlags(env["BUILD_FLAGS"])
    device_name = [build_flag for build_flag in build_flags.get("CPPDEFINES")][0]
    device_name = device_name.lower().capitalize()
    build_date = datetime.datetime.today().strftime("%Y-%m-%d")

    # Select output folder and header file by device variant
    if device_name == "Fleurie":
        html_gen_dir_variant = "data_gen_fleurie"
        header_file_variant = "lib/Network/html_pages_fleurie.h"
    else:
        html_gen_dir_variant = "data_gen_pixie"
        header_file_variant = "lib/Network/html_pages_pixie.h"

    # Read out specific html chunks to be replaced in the generic html file
    with open(os.path.join(html_input_dir, "chunk_form_fleurie.html"), "r") as f:
        form_fleurie = f.read()
    with open(os.path.join(html_input_dir, "chunk_form_pixie.html"), "r") as f:
        form_pixie = f.read()
    with open(os.path.join(html_input_dir, "chunk_navbar.html"), "r") as f:
        navbar = f.read()
    with open(os.path.join(html_input_dir, "chunk_footer.html"), "r") as f:
        footer = f.read()

    # Build processed HTML content map before hashing
    processed_html = {}
    for filename in sorted(os.listdir(html_input_dir)):
        if filename.startswith("chunk_"):
            continue

        with open(os.path.join(html_input_dir, filename), "r", encoding="utf-8") as f:
            file_data = f.read()

        if filename.endswith(".html"):
            file_data = str_replace(file_data, "%NAVBAR%", navbar)
            file_data = str_replace(file_data, "%FOOTER%", footer)
            file_data = str_replace(file_data, "%DEV_NAME%", device_name)
            file_data = str_replace(file_data, "%BUILD_DATE%", build_date)
            if device_name == "Fleurie":
                file_data = str_replace(file_data, "%LIGHT_FORM%", form_fleurie)
            elif device_name == "Pixie":
                file_data = str_replace(file_data, "%LIGHT_FORM%", form_pixie)

        processed_html[filename] = file_data

    # Hash after chunk replacements and to be consistent with what is written.
    hasher = hashlib.sha256()
    for filename in sorted(processed_html):
        hasher.update(filename.encode("utf-8"))
        hasher.update(processed_html[filename].encode("utf-8"))
    hasher.update(device_name.encode("utf-8"))
    current_hash = hasher.hexdigest()

    # Quick early skip if header already generated from same input set
    if os.path.exists(header_file_variant):
        with open(header_file_variant, "r", encoding="utf-8") as hf:
            hf.readline()  # skip comment line
            hf.readline()  # skip BuildDate line
            hash_line = hf.readline().strip()

        if hash_line.startswith("// ContentHash:"):
            old_hash = hash_line.split(":", 1)[1].strip()
            if old_hash == current_hash:
                print(f"PRE SCRIPT: {header_file_variant} already up-to-date, skipping generation.")
                return

    # Clear the header file and include metadata for skip checks
    with open(header_file_variant, "w", encoding="utf-8") as f:
        f.write("/* THIS HEADER IS GENERATED BY SCRIPT. DO NOT MODIFY! */\n")
        f.write(f"// BuildDate: {build_date}\n")
        f.write(f"// ContentHash: {current_hash}\n\n")

    # Write processed data to header + generated directory
    for filename in sorted(processed_html):
        file_data = processed_html[filename]

        compressed_data = gzip.compress(file_data.encode())
        write_compressed_data_to_header(filename, compressed_data, header_file_variant)

        # Write to variant-specific output directory
        write_to_file(html_gen_dir_variant, filename, file_data)


generate_html_pages(None, None, env)

