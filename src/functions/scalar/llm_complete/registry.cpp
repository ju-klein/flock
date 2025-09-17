#include "flockmtl/registry/registry.hpp"

//#include "../../../../duckdb/src/include/duckdb/main/extension/extension_loader.hpp" TODO: FIX THESE, THESE ARE AUTOMATIC!
#include "flockmtl/functions/scalar/llm_complete.hpp"

namespace flockmtl {

void ScalarRegistry::RegisterLlmComplete(duckdb::ExtensionLoader& loader) {
    loader.RegisterFunction(
            duckdb::ScalarFunction("llm_complete",
                                       {duckdb::LogicalType::ANY, duckdb::LogicalType::ANY},
                                       duckdb::LogicalType::JSON(), LlmComplete::Execute));
}

}// namespace flockmtl
