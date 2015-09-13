//
// Created by Tobias Conradi on 30.07.15.
// Licensed under MIT License, see LICENSE for full text.

#define BOOST_TEST_MODULE ModelTest
#include <boost/test/included/unit_test.hpp>
#include <Matching/Matcher/DependenceGraphMatching.h>
#include <Model/PatternPersistence.h>
#include <Standalone/DumpDisassemblerAPI.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
#include <boost/graph/graphviz.hpp>


std::shared_ptr<IdiomMatcher::JSONValue> patternJSON();
std::shared_ptr<IdiomMatcher::JSONValue> disassemblyJSON();

BOOST_AUTO_TEST_CASE(TestSpecificMatchFailure) {
    using namespace IdiomMatcher;
    DependenceGraphMatching matching;

    auto pattern = patternFromJSON(*patternJSON());
    auto document = documentFromJSON(*disassemblyJSON());
    DumpDisassemblerAPI api(document,"");
    auto startEA = api.minEA();
    Matching::ExtractedValuesMap valuesMap;
	Patterns patterns;
	patterns.push_back(pattern);

	bool matched = false;
	IdiomMatcher::Matching::FoundMatchFunctionCallback callback = [&matched](const IdiomMatcher::Pattern &pattern, const IdiomMatcher::EA &startEA, const IdiomMatcher::EA &endEA, const std::map<std::string,std::string>& extractedValues) -> bool {
		matched=true;
		std::cout << "matched pattern " << pattern.getName() <<  std::endl;
		return true;
	};

	matching.testForPatternsStartingAtEA(patterns,startEA,api, callback);
    BOOST_CHECK(matched);
}

std::shared_ptr<IdiomMatcher::JSONValue> patternJSON() {
    using namespace rapidjson;
    const char* json = "{\"instructions\": [{\"mnem\": \"movzx\",\"ops\": [{\"modified\": true,\"nameIsTemplate\": true,\"regs\": [\"edx\"],\"text\": \"edx\",\"used\": false},{\"nameIsTemplate\": true,\"regs\": [\"ax\"],\"text\": \"ax\"}],\"size\": 3,\"xrefs\": [{\"target\": 135234381}]},{\"mnem\": \"cmp\",\"ops\": [{\"regs\": [\"ax\"],\"nameIsTemplate\": true,\"text\": \"ax\"},{\"text\": \"1Ah\"}],\"size\": 4,\"xrefs\": [{\"target\": 135234385}]},{\"mnem\": \"ja\",\"ops\": [{\"address\": 135234381,\"text\": \"loc_80F834D\"}],\"size\": 2,\"xrefs\": [{\"target\": 135234387},{\"target\": 135234381}]},{\"mnem\": \"jmp\",\"ops\": [{\"address\": 137237980,\"regs\": [\"edx\"],\"nameIsTemplate\": true,\"text\": \"ds:off_82E15DC[edx*4]\"}],\"size\": 7,\"xrefs\": [{\"target\": 135234381},{\"target\": 135234448},{\"target\": 135234512},{\"target\": 135234568},{\"target\": 135234632},{\"target\": 135234744},{\"target\": 135234800},{\"target\": 135234824},{\"target\": 135234848},{\"target\": 135234912},{\"target\": 135234952},{\"target\": 135235072},{\"target\": 135235128},{\"target\": 135235192},{\"target\": 135235240},{\"target\": 135235328},{\"target\": 135235392},{\"target\": 135235504},{\"isData\": true,\"target\": 137237980}]}],\"name\": \"switch movzx before\"}";

    typedef GenericDocument<ASCII<>, MemoryPoolAllocator<>, MemoryPoolAllocator<>> DocumentType;
    auto d = std::make_shared<DocumentType>();
    d->Parse(json);
    return d;
};

std::shared_ptr<IdiomMatcher::JSONValue> disassemblyJSON() {
    using namespace rapidjson;

	const char* json = "{\"version\":2,\"binaryName\":\"Xalan_base.foobar\",\"architecture\":\"metapc\\u0000\\u0000\",\"disassembler\":\"IDA Pro\",\"minEA\":134519104,\"maxEA\":138521748,\"disassembly\":[{\"ea\":134519104,\"instruction\":{\"mnem\":\"movzx\",\"size\":3,\"ops\":[{\"text\":\"edx\",\"regs\":[\"edx\"],\"used\":false,\"modified\":true},{\"text\":\"ax\",\"regs\":[\"ax\"]}],\"xrefs\":[{\"target\":135234381}]}},{\"ea\":135234381,\"instruction\":{\"mnem\":\"cmp\",\"size\":4,\"ops\":[{\"text\":\"ax\",\"regs\":[\"ax\"]},{\"text\":\"1Ah\"}],\"xrefs\":[{\"target\":135234385}]},\"comment\":\"switch 27 cases \"},{\"ea\":135234385,\"instruction\":{\"mnem\":\"ja\",\"size\":2,\"ops\":[{\"text\":\"loc_80F834D\",\"address\":135234381}],\"xrefs\":[{\"target\":135234387},{\"target\":135234381}]}},{\"ea\":135234387,\"instruction\":{\"mnem\":\"jmp\",\"size\":7,\"ops\":[{\"text\":\"ds:off_82E15DC[edx*4]\",\"regs\":[\"edx\"],\"address\":137237980}],\"xrefs\":[{\"target\":135234381},{\"target\":135234448},{\"target\":135234512},{\"target\":135234568},{\"target\":135234632},{\"target\":135234744},{\"target\":135234800},{\"target\":135234824},{\"target\":135234848},{\"target\":135234912},{\"target\":135234952},{\"target\":135235072},{\"target\":135235128},{\"target\":135235192},{\"target\":135235240},{\"target\":135235328},{\"target\":135235392},{\"target\":135235504},{\"target\":137237980,\"isData\":true}]},\"comment\":\"switch jump\"},{\"ea\":135234394,\"instruction\":{\"mnem\":\"movsx\",\"size\":5,\"ops\":[{\"text\":\"eax\",\"regs\":[\"eax\"],\"used\":false,\"modified\":true},{\"text\":\"word ptr [esp+4Ch+var_20]\",\"regs\":[\"sp\"],\"address\":44}],\"xrefs\":[{\"target\":135234399}]}}]}";

// deleted loop xrefs
/*
	const char* json = "{\"version\":2,\"binaryName\":\"Xalan_base.foobar\",\"architecture\":\"metapc\\u0000\\u0000\",\"disassembler\":\"IDA Pro\",\"minEA\":134519104,\"maxEA\":138521748,\"disassembly\":[{\"ea\":134519104,\"instruction\":{\"mnem\":\"movzx\",\"size\":3,\"ops\":[{\"text\":\"edx\",\"regs\":[\"edx\"],\"used\":false,\"modified\":true},{\"text\":\"ax\",\"regs\":[\"ax\"]}],\"xrefs\":[{\"target\":135234381}]}},{\"ea\":135234381,\"instruction\":{\"mnem\":\"cmp\",\"size\":4,\"ops\":[{\"text\":\"ax\",\"regs\":[\"ax\"]},{\"text\":\"1Ah\"}],\"xrefs\":[{\"target\":135234385}]},\"comment\":\"switch 27 cases \"},{\"ea\":135234385,\"instruction\":{\"mnem\":\"ja\",\"size\":2,\"ops\":[{\"text\":\"loc_80F834D\",\"address\":135234381}],\"xrefs\":[{\"target\":135234387},{\"target\":135234382}]}},{\"ea\":135234387,\"instruction\":{\"mnem\":\"jmp\",\"size\":7,\"ops\":[{\"text\":\"ds:off_82E15DC[edx*4]\",\"regs\":[\"edx\"],\"address\":137237980}],\"xrefs\":[{\"target\":135234448},{\"target\":135234512},{\"target\":135234568},{\"target\":135234632},{\"target\":135234744},{\"target\":135234800},{\"target\":135234824},{\"target\":135234848},{\"target\":135234912},{\"target\":135234952},{\"target\":135235072},{\"target\":135235128},{\"target\":135235192},{\"target\":135235240},{\"target\":135235328},{\"target\":135235392},{\"target\":135235504},{\"target\":137237980,\"isData\":true}]},\"comment\":\"switch jump\"},{\"ea\":135234394,\"instruction\":{\"mnem\":\"movsx\",\"size\":5,\"ops\":[{\"text\":\"eax\",\"regs\":[\"eax\"],\"used\":false,\"modified\":true},{\"text\":\"word ptr [esp+4Ch+var_20]\",\"regs\":[\"sp\"],\"address\":44}],\"xrefs\":[{\"target\":135234399}]}}]}";
*/
/*
const char *json = "{\"version\":2,\"binaryName\":\"Xalan_base.foobar\",\"architecture\":\"metapc\\u0000\\u0000\",\"disassembler\":\"IDA Pro\",\"minEA\":134519104,\"maxEA\":135234394,\"disassembly\":[{\"ea\":134519104,\"instruction\":{\"mnem\":\"movzx\",\"size\":3,\"ops\":[{\"text\":\"edx\",\"regs\":[\"edx\"],\"used\":false,\"modified\":true},{\"text\":\"ax\",\"regs\":[\"ax\"]}],\"xrefs\":[{\"target\":135234381}]}},{\"ea\":135234381,\"instruction\":{\"mnem\":\"cmp\",\"size\":4,\"ops\":[{\"text\":\"ax\",\"regs\":[\"ax\"]},{\"text\":\"1Ah\"}],\"xrefs\":[{\"target\":135234385}]},\"comment\":\"switch 27 cases \"},{\"ea\":135234385,\"instruction\":{\"mnem\":\"ja\",\"size\":2,\"ops\":[{\"text\":\"loc_80F834D\",\"address\":135234381}],\"xrefs\":[{\"target\":135234387},{\"target\":135234381,\"isUnordinaryFlow\":true}]}},{\"ea\":135234387,\"instruction\":{\"mnem\":\"jmp\",\"size\":7,\"ops\":[{\"text\":\"ds:off_82E15DC[edx*4]\",\"regs\":[\"edx\"],\"address\":137237980}],\"xrefs\":[{\"target\":135234381,\"isUnordinaryFlow\":true},{\"target\":135234448,\"isUnordinaryFlow\":true},{\"target\":135234512,\"isUnordinaryFlow\":true},{\"target\":135234568,\"isUnordinaryFlow\":true},{\"target\":135234632,\"isUnordinaryFlow\":true},{\"target\":135234744,\"isUnordinaryFlow\":true},{\"target\":135234800,\"isUnordinaryFlow\":true},{\"target\":135234824,\"isUnordinaryFlow\":true},{\"target\":135234848,\"isUnordinaryFlow\":true},{\"target\":135234912,\"isUnordinaryFlow\":true},{\"target\":135234952,\"isUnordinaryFlow\":true},{\"target\":135235072,\"isUnordinaryFlow\":true},{\"target\":135235128,\"isUnordinaryFlow\":true},{\"target\":135235192,\"isUnordinaryFlow\":true},{\"target\":135235240,\"isUnordinaryFlow\":true},{\"target\":135235328,\"isUnordinaryFlow\":true},{\"target\":135235392,\"isUnordinaryFlow\":true},{\"target\":135235504,\"isUnordinaryFlow\":true},{\"target\":137237980,\"isData\":true,\"isUnordinaryFlow\":true}]},\"comment\":\"switch jump\"},{\"ea\":135234394,\"instruction\":{\"mnem\":\"movsx\",\"size\":5,\"ops\":[{\"text\":\"eax\",\"regs\":[\"eax\"],\"used\":false,\"modified\":true},{\"text\":\"word ptr [esp+4Ch+var_20]\",\"regs\":[\"sp\"],\"address\":44}],\"xrefs\":[{\"target\":135234399}]}}]}";
*/
	typedef GenericDocument<ASCII<>, MemoryPoolAllocator<>, MemoryPoolAllocator<>> DocumentType;
    auto d = std::make_shared<DocumentType>();
    d->Parse(json);
    return d;
}


typedef boost::directed_graph<std::string> StringGraph;

struct vertex_writer {
public:
	vertex_writer(const StringGraph &graph) : _graph(&graph) {}
	void operator()(std::ostream& out, const StringGraph::vertex_descriptor& v) const {
		out << "[label=\"" << (*_graph)[v] << "\"]";
	}
private:
	const StringGraph *_graph;
};

void writeGraphToGraphViz(StringGraph &graph) {
	write_graphviz(std::cout,graph,vertex_writer(graph));
}

BOOST_AUTO_TEST_CASE(TestSpecificVF2Failure) {

	using namespace boost;
	typedef boost::directed_graph<std::string> StringGraph;
	typedef boost::graph_traits<StringGraph> StringGraphTraits;
	typedef StringGraph::vertex_descriptor StringGraphVertexDescriptor;
	typedef StringGraph::edge_descriptor StringGraphEdgeDescriptor;

	StringGraph g;
	auto v0 = g.add_vertex("A");
	auto v1 = g.add_vertex("B");
	auto v2 = g.add_vertex("C");
	auto v3 = g.add_vertex("D");
	auto v4 = g.add_vertex("S");

//	auto v5 = g.add_vertex("E");

	g.add_edge(v4, v0);
	g.add_edge(v4, v1);
	g.add_edge(v4, v2);
	g.add_edge(v0, v2);
	g.add_edge(v1, v2);
	g.add_edge(v2, v3);
//	g.add_edge(v2, v1); // additional
	g.add_edge(v3, v1); // additional
//	g.add_edge(v3, v2); // additional

//	g.add_edge(v3, v5);
//	g.add_edge(v5, v1);

	StringGraph s;
	auto u0 = s.add_vertex("A");
	auto u1 = s.add_vertex("B");
	auto u2 = s.add_vertex("C");
	auto u3 = s.add_vertex("D");
	auto u4 = s.add_vertex("S");

	s.add_edge(u4, u0);
	s.add_edge(u4, u1);
	s.add_edge(u4, u2);
	s.add_edge(u0, u2);
	s.add_edge(u1, u2);
	s.add_edge(u2, u3);


	auto &small = s;
	auto &large = g;
	// Create callback
	vf2_print_callback<StringGraph, StringGraph> callback(small, large);
	bool matched = vf2_subgraph_mono(small,
									large,
									callback,
									get(vertex_index, small),
									get(vertex_index, large),
									vertex_order_by_mult(small),
									[this, &small, &large](StringGraphEdgeDescriptor small_edge, StringGraphEdgeDescriptor large_edge) {

										auto source1 = small[source(small_edge,small)];
										auto source2 = large[source(large_edge,large)];
										if (source1 != source2)
											return false;
										auto target1 = small[target(small_edge,small)];
										auto target2 = large[target(large_edge,large)];
										return target1 == target2;
									},
									[this, &small, &large](StringGraphVertexDescriptor small_vd, StringGraphVertexDescriptor large_vd) {
										auto u = small[small_vd];
										auto v = large[large_vd];
										return u == v;
									});


	writeGraphToGraphViz(small);
	writeGraphToGraphViz(large);

	BOOST_CHECK(matched);
}
