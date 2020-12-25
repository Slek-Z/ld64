
BYTES_PER_LINE = 12
hexbytes = list()

with open("LD64.BIN", "rb") as f:
  byte = f.read(1)
  while byte:
    hexbytes.append("0x" + byte.hex().upper())
    if len(hexbytes) == BYTES_PER_LINE:
      print("\".byte " + ", ".join(hexbytes) + ";\"")
      hexbytes.clear()
    byte = f.read(1)
  if hexbytes:
      print("\".byte " + ", ".join(hexbytes) + ";\"")
      hexbytes.clear()
