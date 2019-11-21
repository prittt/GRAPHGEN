if(GRAPHGEN_FREQUENCIES_ENABLED)
	target_sources(GRAPHGEN PRIVATE
		image_frequencies.cpp
		image_frequencies.h
	)
endif()

target_sources(GRAPHGEN PRIVATE

    base_ruleset.h
    collect_drag_stats.h
	conact_code_generator.h
	conact_tree.h    
	condition_action.h
    config_data.h
	connectivity_graph.h
	connectivity_mat.h
    drag.h
    drag_compressor.h
    drag_statistics.h
	drag2optimal.h    
    end_forest.h
    find_optimal_drag.h
    forest.h
    forest_handler.h
	forest_optimizer.h
    forest_statistics.h
    forest2dag.h
    graphgen.h
    graph_code_generator.h
	hypercube.h
	merge_set.h
	output_generator.h
	performance_evaluator.h
	pixel_set.h
	remove_equal_subtrees.h
    rule_set.h
    system_info.h
    tree.h
	tree2dag_identities.h
	utilities.h

    queue.h
    semaphore.h
    pool.h

	conact_code_generator.cpp    
    conact_tree.cpp
    config_data.cpp
	connectivity_graph.cpp
	drag2optimal.cpp
    drag_statistics.cpp
	forest.cpp
	forest2dag.cpp
	graph_code_generator.cpp
	hypercube.cpp
	output_generator.cpp
	tree2dag_identities.cpp
	utilities.cpp   
   
)
