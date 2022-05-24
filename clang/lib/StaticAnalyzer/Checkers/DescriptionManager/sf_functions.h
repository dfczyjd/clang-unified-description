// Enum for setting VarType parameter for GenericTaintChecker 
enum class SF_TaintCheckerVarType { None, Src, Dst };

// Marks its arguments as sinks for GenericTaintChecker
template <typename... Variable>
void sf_sink(Variable...) {}

// Marks its arguments as filters for GenericTaintChecker
template <typename... Variable>
void sf_filter(Variable...) {}

// Marks its arguments as propagation sources for GenericTaintChecker
template <typename... Variable>
void sf_propagation_src(Variable...) {}

// Marks its arguments as propagation destinations for GenericTaintChecker
template <typename... Variable>
void sf_propagation_dst(Variable...) {}

// Marks the return value as propagation destination for GenericTaintChecker
void sf_propagation_return() {}

// Marks the current function's variadic argument under the given index as
// source or desctination (depending on enum value given) for GenericTaintChecker
void sf_propagation_variadic(SF_TaintCheckerVarType type, int index) {}

// Enum for setting AllocationFamily parameters for MallocChecker
enum class SF_AllocationFamily {
  AF_None,
  AF_Malloc,
  AF_CXXNew,
  AF_CXXNewArray,
  AF_IfNameIndex,
  AF_Alloca,
  AF_InnerBuffer
};

// Marks that the function allocates memory block of given size.
// The pointer to the allocated block is the return value
void* sf_allocate_with_size(size_t size, SF_AllocationFamily) { return nullptr; }
