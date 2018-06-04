from crypto import AES
import crypto


class Ccrypto:
  def __init__(self,key):
      self.key=key
  def crypt(self,data):
    iv = crypto.getrandbits(128) # hardware generated random IV (never reuse it)
    cipher = AES(self.key, AES.MODE_CFB, iv)
    msg = iv + cipher.encrypt(data)
    print (msg)
    return msg
  def decrypt(self,data):
    iv = crypto.getrandbits(128) # hardware generated random IV (never reuse it)
    cipher = AES(self.key, AES.MODE_CFB, data[:16]) # on the decryption side
    original = cipher.decrypt(data[16:])
    print(original)
    return original
