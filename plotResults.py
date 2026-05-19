import matplotlib as plt

# Open the file in read mode
file = open("out.txt", "r")

# Read the entire content of the file
content = file.read()
print(content)

# Close the file
file.close()