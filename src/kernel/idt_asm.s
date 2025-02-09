.section .text
.global load_idt

# Function to load the IDT
# Parameter: pointer to IDT descriptor
load_idt:
    pushl %ebp           # Save old base pointer
    movl %esp, %ebp      # Create new stack frame
    movl 8(%ebp), %eax   # Get pointer to IDT descriptor
    lidt (%eax)          # Load IDT
    movl %ebp, %esp      # Restore stack frame
    popl %ebp           # Restore old base pointer
    ret                  # Return to caller
