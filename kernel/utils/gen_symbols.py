# Initialize the symbol file
symfile = ""

# Open the output file
o = open('bin/symfile.s','w') 

# Open the symbol file
with open('bin/kernel.sym','r') as f:
    symfile = f.readlines()

# 32 bit assembly
o.write("[BITS 32]\n")
o.write("SECTION .symbols\n")

# Parse the file line by line
for line in symfile:
    # Split the symbol
    sym_addr = line.split(' ')[0]
    sym_name = (line.split(' ')[1])[:-1]

    # Remove the leading 10 characters, in order to turn the 64bit address in 32bit format.
    sym_addr = sym_addr[10:]

    # Write the symbol definition
    o.write("dd 0x" + sym_addr + '\n')
    o.write("dd " + sym_name + '\n')

# Write the terminating symbol (Both entries null)
o.write("dd 0x0" + '\n')
o.write("dd 0x0" + '\n')

for line in symfile:
    # Split the symbol
    sym_addr = line.split(' ')[0]
    sym_name = (line.split(' ')[1])[:-1]

    # Remove the leading 10 characters, in order to turn the 64bit address in 32bit format.
    sym_addr = sym_addr[10:]

    # Write the symbol definition
    o.write(sym_name + ":" + '\n')
    o.write("db \"" + sym_name + "\", 0\n")

o.close()

