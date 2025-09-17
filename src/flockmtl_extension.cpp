#define DUCKDB_EXTENSION_MAIN

#include "flockmtl_extension.hpp"

#include "duckdb/parser/parser.hpp"
#include "duckdb/parser/statement/extension_statement.hpp"
#include "flockmtl/core/common.hpp"
#include "flockmtl/core/config.hpp"
#include "flockmtl/custom_parser/query_parser.hpp"

#include <flockmtl/model_manager/model.hpp>

namespace duckdb {

static void LoadInternal(ExtensionLoader& loader) { /* this code snippet taken from https://github.com/duckdb/duckdb/pull/17772#issue-3113786872, 17-09-2025-1-55-pm. */
    DatabaseInstance& instance = loader.GetDatabaseInstance();
    flockmtl::Config::Configure(loader);

    // Register the custom parser
    auto& config = DBConfig::GetConfig(instance);
    DuckParserExtension duck_parser;
    config.parser_extensions.push_back(duck_parser);
    config.operator_extensions.push_back(make_uniq<DuckOperatorExtension>());
}

ParserExtensionParseResult duck_parse(ParserExtensionInfo*, const std::string& query) {
    flockmtl::QueryParser query_parser;

    // Translate and print SQL queries for each input query
    std::string sql_query = query_parser.ParseQuery(query);

    // Parse and return the statement using DuckDB's parser
    Parser parser;
    parser.ParseQuery(sql_query);
    auto statements = std::move(parser.statements);

    return ParserExtensionParseResult(
        make_uniq_base<ParserExtensionParseData, DuckParseData>(std::move(statements[0])));
}

ParserExtensionPlanResult duck_plan(ParserExtensionInfo*, ClientContext& context,
                                    unique_ptr<ParserExtensionParseData> parse_data) {
    if (auto state = context.registered_state->Get<DuckState>("duck")) {
        context.registered_state->Remove("duck");
    }
    context.registered_state->GetOrCreate<DuckState>("duck", std::move(parse_data));
    throw BinderException("Use duck_bind instead");
}

BoundStatement duck_bind(ClientContext& context, Binder& binder, OperatorExtensionInfo* info, SQLStatement& statement) {
    switch (statement.type) {
    case StatementType::EXTENSION_STATEMENT: {
        auto& extension_statement = dynamic_cast<ExtensionStatement&>(statement);
        if (extension_statement.extension.parse_function == duck_parse) {
            if (const auto duck_state = context.registered_state->Get<DuckState>("duck")) {
                const auto duck_binder = Binder::CreateBinder(context, &binder);
                const auto duck_parse_data = dynamic_cast<DuckParseData*>(duck_state->parse_data.get());
                auto bound_statement = duck_binder->Bind(*(duck_parse_data->statement));
                BoundStatement result;
                return bound_statement;
            }
            throw BinderException("Registered state not found");
        }
    }
    default:
        // No-op empty
        return {};
    }
}

void FlockmtlExtension::Load(ExtensionLoader &loader) { LoadInternal(loader); } /* this code snippet taken from https://github.com/duckdb/duckdb/pull/17772#issue-3113786872, 17-09-2025-1-55-pm. */

std::string FlockmtlExtension::Name() { return "flockmtl"; }
std::string FlockmtlExtension::Version() const {
#ifdef EXT_VERSION_FLOCKMTL
    return EXT_VERSION_FLOCKMTL;
#else
    return "";
#endif
}

} // namespace duckdb

DUCKDB_CPP_EXTENSION_ENTRY(flockmtl, loader) {  /* this code snippet taken from https://github.com/duckdb/duckdb/pull/17772#issue-3113786872, 17-09-2025-1-55-pm. naming this "loader" taken from core_functions_extension.cpp */
    duckdb::LoadInternal(loader);
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
