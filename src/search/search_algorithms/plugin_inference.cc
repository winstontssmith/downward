#include "inference.h"
#include "search_common.h"

#include "../plugins/plugin.h"

using namespace std;

namespace plugin_inference {
    class InferenceFeature
        : public plugins::TypedFeature<SearchAlgorithm, inference_search::InferenceSearch> {
    public:
        InferenceFeature() : TypedFeature("inference") {
            document_title("Lazy inference best-first search");
            document_synopsis("");

            add_option<shared_ptr<OpenListFactory>>("open", "open list");
            add_option<bool>("reopen_closed", "reopen closed nodes", "false");
            add_list_option<shared_ptr<Evaluator>>(
                "preferred",
                "use preferred operators of these evaluators", "[]");
            add_successors_order_options_to_feature(*this);
            add_search_algorithm_options_to_feature(*this, "inference");
        }

        virtual shared_ptr<inference_search::InferenceSearch>
            create_component(const plugins::Options& opts) const override {
            return plugins::make_shared_from_arg_tuples<inference_search::InferenceSearch>(
                opts.get<shared_ptr<OpenListFactory>>("open"),
                opts.get<bool>("reopen_closed"),
                opts.get_list<shared_ptr<Evaluator>>("preferred"),
                get_successors_order_arguments_from_options(opts),
                get_search_algorithm_arguments_from_options(opts)
            );
        }
    };

    static plugins::FeaturePlugin<InferenceFeature> _plugin;
}
