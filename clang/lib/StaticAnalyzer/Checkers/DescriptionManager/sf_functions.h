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

// Sets buffer size for the given array
void sf_buffer_size(void* array, size_t size) {}
