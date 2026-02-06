import re
import os

version_file = "infinity-tag-esp32/include/Version.h"

def increment_build_number(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    # Regex to find #define VERSION_BUILD X
    pattern = r'(#define\s+VERSION_BUILD\s+)(\d+)'
    
    match = re.search(pattern, content)
    if match:
        prefix = match.group(1)
        current_build = int(match.group(2))
        new_build = current_build + 1
        
        # User requirement: 0-99. But usually build numbers just grow. 
        # User said "each digit 0-99". 
        # "Fourth digit +1... only modify fourth digit".
        # If it hits 100, should we wrap? The user request implies "v1.0.0.0" format. 
        # "Every bit can be 0~99". 
        if new_build > 99:
            new_build = 0 # Wrap around if strictly limited to 2 digits, or let it grow?
            # "Only modify 4th digit". If I carry over to patch, I violate the rule.
            # So wrapping to 0 seems safest to stick to "0-99" rule strictly.
            print(f"Build number wrapped from 99 to 0")

        new_content = re.sub(pattern, f'{prefix}{new_build}', content)
        
        with open(file_path, 'w') as f:
            f.write(new_content)
        
        print(f"Updated VERSION_BUILD to {new_build}")
    else:
        print("VERSION_BUILD definition not found!")

if __name__ == "__main__":
    # Adjust path if running from project root
    if not os.path.exists(version_file):
        # Try finding it relative to current script if executed from elsewhere
        # Assuming script is in project_root/tools/
        # But for now, let's assume we run from project root
        pass
        
    increment_build_number(version_file)
