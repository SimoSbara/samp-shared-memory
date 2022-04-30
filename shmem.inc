// Natives

native OpenShMemory(const memName[], memSize);
native DestroyShMemory(const memName[]);
native GetShMemoryData(const memName[], dest[]);
native SetShMemoryData(const memName[], input[], copySize, startOffset = 0);
native GetShMemorySize(const memName[]);
