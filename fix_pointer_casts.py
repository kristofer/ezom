#!/usr/bin/env python3
"""
Script to fix pointer casting issues in EZOM source code.
Replaces direct pointer casts with EZOM_OBJECT_PTR() macro.
"""

import re
import os
import glob

def fix_pointer_casts(content):
    """Fix pointer casting patterns in the content."""
    
    # Define the struct types that need fixing
    struct_types = [
        'ezom_object_t',
        'ezom_class_t', 
        'ezom_integer_t',
        'ezom_string_t',
        'ezom_symbol_t',
        'ezom_method_dict_t',
        'ezom_array_t',
        'ezom_block_t',
        'ezom_context_t',
        'ezom_method_t'
    ]
    
    changes_made = 0
    
    # For each struct type, find and replace the casting pattern
    for struct_type in struct_types:
        # Pattern: (ezom_struct_t*)variable
        pattern = r'\(' + re.escape(struct_type) + r'\*\)([a-zA-Z_][a-zA-Z0-9_]*(?:\[[^\]]*\])?(?:\->[a-zA-Z_][a-zA-Z0-9_]*)*)'
        
        def replace_cast(match):
            variable = match.group(1)
            return f'({struct_type}*)EZOM_OBJECT_PTR({variable})'
        
        new_content, count = re.subn(pattern, replace_cast, content)
        if count > 0:
            print(f"  Fixed {count} instances of {struct_type} casts")
            changes_made += count
            content = new_content
    
    return content, changes_made

def process_file(filepath):
    """Process a single file."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
        
        new_content, changes = fix_pointer_casts(content)
        
        if changes > 0:
            print(f"Processing {filepath}: {changes} changes")
            with open(filepath, 'w') as f:
                f.write(new_content)
            return changes
        else:
            print(f"Processing {filepath}: no changes needed")
            return 0
            
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return 0

def main():
    """Main function."""
    print("Fixing pointer casting issues in EZOM source code...")
    
    # Files to process
    files_to_process = [
        'vm/src/bootstrap.c',
        'vm/src/objects.c',
        'vm/src/object.c',
        'vm/src/context.c',
        'vm/src/dispatch.c',
        'vm/src/primitives.c',
        'vm/src/evaluator.c',
        'vm/src/main.c'
    ]
    
    total_changes = 0
    
    for filepath in files_to_process:
        if os.path.exists(filepath):
            changes = process_file(filepath)
            total_changes += changes
        else:
            print(f"File not found: {filepath}")
    
    print(f"\nTotal changes made: {total_changes}")
    
    if total_changes > 0:
        print("\nPointer casting fixes completed!")
        print("Please rebuild the project to test the fixes.")
    else:
        print("\nNo changes were needed.")

if __name__ == "__main__":
    main()