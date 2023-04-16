Import("env")
import os
import gzip
import htmlmin

# Path to the output C header file
header_file_path = 'lib/Network/html_pages.h'

# Path to the generated html files
html_gen_dir = 'data_gen'

# Path to the HTML file you want to compress
html_input_dir = 'data'

def str_replace(s, search, replacement):
    return s.replace(search, replacement)

def write_compressed_data_to_header(filename, compressed_data):
    var_name = filename.replace('.', '_')
    # Write the name of the compressed data variable
    with open(header_file_path, 'a') as f:
        f.write('const uint8_t {}[] PROGMEM = {{\n'.format(var_name))

        # Write the compressed data as a byte array
        for i, byte in enumerate(compressed_data):
            if i != 0:
                if i % 16 == 0:
                    f.write(',\n')
                else:
                    f.write(', ')
            f.write('0x{:02x}'.format(byte))
        f.write('\n};\n')

        # Write the constant with the size of the compressed data
        f.write('const size_t {}_size = {};\n\n'.format(var_name, len(compressed_data)))

def write_string_data_to_header(filename, data):
    var_name = filename.replace('.', '_')
    # Generate the C header file containing the HTML data as a char array
    with open(header_file_path, 'a', encoding='utf-8') as f:
        compressed_code = htmlmin.minify(data, remove_comments=True)
        f.write('const char {}[] PROGMEM = R"=====(\n'.format(var_name))
        f.write(compressed_code)
        f.write(')=====";\n\n')

def write_to_file(filename, input_str):
    os.makedirs(html_gen_dir, exist_ok=True)
    filename = os.path.join(html_gen_dir, filename)
    with open(filename, 'w') as f:
        f.write(input_str)





print("\nPRE SCRIPT: Generating web related files and variables...\n")
build_flags = env.ParseFlags(env['BUILD_FLAGS'])
device_name = [build_flag for build_flag in build_flags.get('CPPDEFINES') ][0]
device_name = device_name.lower().capitalize()

# Read out specific html chunks to be replaced in the generic html file
with open(os.path.join(html_input_dir, 'chunk_form_fleurie.html'), 'r') as f:
    form_fleurie = f.read()
with open(os.path.join(html_input_dir, 'chunk_form_pixie.html'), 'r') as f:
    form_pixie = f.read()
with open(os.path.join(html_input_dir, 'chunk_navbar.html'), 'r') as f:
    navbar = f.read()

# Clear the header file
with open(header_file_path, 'w') as f:
    f.write('/* THIS HEADER IS GENERATED BY SCRIPT. DO NOT MODIFY! */\n\n')

# Loop through all files in the directory, except the chunks
for filename in os.listdir(html_input_dir):
    if filename.startswith('chunk_'):
        continue

    # Open the file and read its contents
    with open(os.path.join(html_input_dir, filename), 'r', encoding='utf-8') as f:
        file_data = f.read()

    # Change device specific chunks in the webpage
    if filename.endswith('.html'):
        file_data = str_replace(file_data, "%NAVBAR%", navbar)
        file_data = str_replace(file_data, "%DEV_NAME%", device_name)
        if device_name == 'Fleurie':
            file_data = str_replace(file_data, "%LIGHT_FORM%", form_fleurie)
        elif device_name == 'Pixie':
            file_data = str_replace(file_data, "%LIGHT_FORM%", form_pixie)

    # Write the data and size to the header file
    # if filename == 'settings.html':
    #     write_string_data_to_header(filename, file_data)
    # else:
    # Compress the file data using gzip
    compressed_data = gzip.compress(file_data.encode())
    write_compressed_data_to_header(filename, compressed_data)

    # Write the processed files into a generated directory
    write_to_file(filename, file_data)