// Natives

native OpenShMemory(const memName[], memSize);
native GetShMemoryData(const memName[], dest[]);
native SetShMemoryData(const memName[], input[], copySize, startOffset = 0);
native GetShMemorySize(const memName[]);
