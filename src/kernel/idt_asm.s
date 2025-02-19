.section .text
.global load_idt

# Function to load the IDT
# Parameter: pointer to IDT descriptor
load_idt:
    movl 4(%esp), %eax  # Get pointer to IDT descriptor from stack
    lidt (%eax)         # Load IDT
    ret                 # Return to caller
