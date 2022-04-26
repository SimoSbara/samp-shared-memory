import mmap
import numpy as np
import random

if __name__ == '__main__':
#6 * 4 -> 5 int32 of data(4 byte each) + 4 byte for boolean flag (int32)
  mem_size = 6 * 4

  while(True):  
      shmem = mmap.mmap(-1, mem_size ,"memory_name", access= mmap.ACCESS_READ)
      shmem.seek(0)
      buf = shmem.read(mem_size)
      varPawno = np.frombuffer(buf, dtype=np.int32, count=5)
      flagMem = np.frombuffer(buf, dtype=np.int32, count=1, offset=(5 * 4))

      if flagMem == True:
          print("collision in read/write")
          continue
        
      print("array \"var\" from pawno = " + str(varPawno)) 
      shmem.close()
