s = set([])
with open("parse_table.csv", "rb") as f:
    while 1:
        bytes = f.read(1)
        if not bytes:
            break
        s.add(bytes[0])
for i in s:
    print(chr(i), hex(i))
