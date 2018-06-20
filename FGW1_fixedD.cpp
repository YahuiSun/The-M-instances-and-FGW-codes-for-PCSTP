#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <list>
#include <vector>
#include <boost/config.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <tuple>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <boost/heap/pairing_heap.hpp> // pairing_heap uses less memory
#include <typeinfo>
using namespace std;
using namespace boost::heap;


#pragma region 
// define an adjacency list with edge weights
typedef boost::property<boost::edge_weight_t, double> EdgeWeightProperty; // define edge weight property
typedef boost::property<boost::vertex_name_t, double> VertexWeightProperty; // define node weight property; note that: vertex_index_t is not mutable
typedef boost::adjacency_list<boost::setS, boost::vecS,
	boost::undirectedS, VertexWeightProperty, EdgeWeightProperty> graph; // define all the graph properties
typedef boost::graph_traits<graph>::adjacency_iterator AdjacencyIterator;
#pragma endregion define graph property  


#pragma region
struct node {
	int index;
	double priority_value;
}; // define the node in the queue
bool operator<(node const& x, node const& y) {
	return x.priority_value > y.priority_value; // < is the max-heap; > is the mean heap; PriorityQueue is expected to be a max-heap of integer values
} // redefine the operator since there are multiple values in each node
typedef typename pairing_heap<node>::handle_type handle_t; // define the handle type for pairing_heap<node>
#pragma endregion define heaps 2018年4月28日16:12:23


#pragma region  
graph read_data_with_terminals(string file_name) {

	int V_num; // vertex number
	int P_num; // number of positive vertices
	int E_num; // edge number
	int v1;
	int v2;
	double weight;
	graph input_graph; // define the adjacency list of the input graph; there is no need to define the V_num
	string line_content;
	ifstream myfile(file_name); // open the file
	if (myfile.is_open()) // if the file is opened successfully
	{
		while (getline(myfile, line_content)) // read file line by line
		{
			// parse the sting：line_content
			list<string> Parsed_content;
			std::string delimiter = " "; // the delimiter
			size_t pos = 0;
			std::string token;
			while ((pos = line_content.find(delimiter)) != std::string::npos) {
				// find(const string& str, size_t pos = 0) function returns the position of the first occurrence of str in the string, or npos if the string is not found.
				token = line_content.substr(0, pos);
				// The substr(size_t pos = 0, size_t n = npos) function returns a substring of the object, starting at position pos and of length npos
				Parsed_content.push_back(token); // store the subtr to the list
				line_content.erase(0, pos + delimiter.length()); // remove the front substr and the first delimiter
			}
			Parsed_content.push_back(line_content); // store the subtr to the list
			if (!Parsed_content.front().compare("Nodes")) // when it's equal, compare returns 0
			{
				Parsed_content.pop_front();
				V_num = atoi(Parsed_content.front().c_str()); // convert string to int
				for (int i = 0; i < V_num; i++) {
					boost::add_vertex(i, input_graph);
					boost::put(boost::vertex_name_t(), input_graph, i, 0);
				}
			}
			else if (!Parsed_content.front().compare("Edges"))
			{
				Parsed_content.pop_front();
				E_num = atoi(Parsed_content.front().c_str());
			}
			else if (!Parsed_content.front().compare("E"))
			{
				Parsed_content.pop_front(); // remove E, expose v1
				v1 = atoi(Parsed_content.front().c_str()) - 1;
				Parsed_content.pop_front(); // remove v1, expose v2
				v2 = atoi(Parsed_content.front().c_str()) - 1;
				Parsed_content.pop_front(); // remove v2, expose weight
				weight = stof(Parsed_content.front().c_str());
				boost::add_edge(v1, v2, weight, input_graph); // add edge
			}
			else if (!Parsed_content.front().compare("Terminals"))
			{
				Parsed_content.pop_front();
				P_num = atoi(Parsed_content.front().c_str());
			}
			else if (!Parsed_content.front().compare("TP"))
			{
				Parsed_content.pop_front(); // remove TP, expose v1
				v1 = atoi(Parsed_content.front().c_str()) - 1;
				Parsed_content.pop_front(); // remove v1, expose weight
				boost::put(boost::vertex_name_t(), input_graph, v1, stof(Parsed_content.front().c_str()));
			}
			else if (!Parsed_content.front().compare("CTP"))
			{
				Parsed_content.pop_front(); // remove CTP, expose v1
				v1 = atoi(Parsed_content.front().c_str()) - 1;
				boost::put(boost::vertex_name_t(), input_graph, v1, 1e8);  // give terminals big prize
			}
		}

		// check number of vertices
		std::cout << "|V|= " << num_vertices(input_graph);
		std::cout << "  |P|= " << P_num;
		// check number of edges
		std::cout << "  |E|= " << num_edges(input_graph) << endl;
		// print errors
		if (V_num != num_vertices(input_graph)) {
			std::cout << "Error: the number of the input vertices is not right." << endl;
		}
		if (E_num != num_edges(input_graph)) {
			std::cout << "Error: the number of the input edges is not right." << endl;
		}
		// connectivity
		std::vector<int> component(num_vertices(input_graph)); // vertex i is in component[i]; No.component from 0
		int cpn_num = connected_components(input_graph, &component[0]); // the number of component; decrease component
		if (cpn_num > 1) {
			std::cout << "cpn_num: " << cpn_num << endl;
			std::cout << "Error: cpn_num>1" << endl;
		}
		return input_graph;

		myfile.close(); //close the file
	}
	else
	{
		std::cout << "Unable to open file " << file_name << endl << "Please check the file location or file name." << endl; // throw an error message
		getchar(); // keep the console window
		exit(1); // end the program
	}
}
#pragma endregion read_data_with_terminals 2016年12月8日


#pragma region

double net_cost(graph input_graph) {

	double included_cost = 0;
	double missed_prize = 0;
	int N = num_vertices(input_graph); // number of vertices
	graph::out_edge_iterator eit1, eend1;

	if (num_edges(input_graph) == 0) {
		double sum_node_weight = get(boost::vertex_name_t(), input_graph, 0);
		double biggest_node_weight = get(boost::vertex_name_t(), input_graph, 0); // initialize as node weight 0
		for (int i = 1; i < N; i++) {
			double x = get(boost::vertex_name_t(), input_graph, i);
			sum_node_weight = sum_node_weight + x;
			if (biggest_node_weight < x) {
				biggest_node_weight = x; // find the maximal node weight
			}
		}
		missed_prize = sum_node_weight - biggest_node_weight;
	}
	else {
		std::vector<int> component(N); // vertex i is in component[i]; No.component from 0
		int cpn_num = connected_components(input_graph, &component[0]); // the number of component; decrease component
		if (cpn_num != N - num_edges(input_graph)) {
			cout << "Solution Error! This is not a simple tree!" << endl;
			getchar();
		}
		for (int i = 0; i < N; i++) {
			if (in_degree(i, input_graph) > 0) { // the included vertex
				tie(eit1, eend1) = boost::out_edges(i, input_graph); // adjacent_vertices of i
				for_each(eit1, eend1,
					[&input_graph, &i, &included_cost](graph::edge_descriptor it1)
				{
					int j = boost::target(it1, input_graph); // the adjacent vetex to i
					if (j > i) { // don't overcount an edge
						included_cost = included_cost + get(boost::edge_weight_t(), input_graph, boost::edge(i, j, input_graph).first); // the edge cost
					}
				});
			}
			else {
				missed_prize = missed_prize + get(boost::vertex_name_t(), input_graph, i);
			}
		}
	}

	return included_cost + missed_prize;
}

#pragma endregion net_cost  2017年4月17日


#pragma region

graph FGW(graph input_graph, double& growth_time, double distribution_ratio) {

	double Global_time = 0; // global time
	int Active_C_num = 0; // the number of active clusters

	int N = num_vertices(input_graph); // number of vertices
	int ep_num = num_edges(input_graph) * 2; // total number of edge parts
	int ep_order = 0;
	node node0;

	// Clusters: the number of clusters is always N
	vector<bool> C_activity(N); // activity value of each C; false means inactive; initial value is false
	vector<double> C_event_time(N); // the event time for each C
	vector<vector<int>> C_V_list(N); // record the vertices in each C
	vector<pairing_heap<node>> C_ep_PQ(N); // the PQ for edge parts in each C; node index: ep order in ep_list
	vector<int> V_locator(N); // the index of the maximal cluster containing the vertex
							  // edge parts: PQ and their handles
	vector<int> ep_v1_list(ep_num); // class may be slow, so I seperate the ep_list
	vector<int> ep_v2_list(ep_num);
	vector<double> ep_EventTime_list(ep_num);
	vector<int> ep_ep2_order_list(ep_num);
	vector<handle_t> handle_ep(ep_num); // store the handle for each edge part
										// the event PQ and their handles
	pairing_heap<node> C_event_PQ; // PQ storing the event time of the active clusters; node index: cluster order
	vector<handle_t> handle_Cevent(N);
	pairing_heap<node> E_event_PQ; // PQ storing the active clusters; node index: cluster order
	vector<handle_t> handle_Eevent(N);

	graph::out_edge_iterator eit, eend;

	// initialize the clusters
	for (int i = 0; i < N; i++)
	{
		C_V_list[i].insert(C_V_list[i].end(), i); // insert a vertex into the rear of C_V_list[i]
		V_locator[i] = i; // the maximal cluster containing vertex i
						  // add edge parts into C
		tie(eit, eend) = boost::out_edges(i, input_graph); // adjacent_vertices of i
		for_each(eit, eend,
			[&input_graph, &ep_v1_list, &ep_v2_list, &ep_ep2_order_list, &handle_ep, &C_ep_PQ, &node0, // the & above is the capture-list: the variables you can use inside
			&ep_order, &i, &ep_EventTime_list, &distribution_ratio](graph::edge_descriptor it) // for each adjacenct vertex boost::target(it, input_graph)
		{
			int j = boost::target(it, input_graph); // the adjacent vetex to i
			if (j > i) { // don't overcheck an edge
						 // the first ep
				ep_v1_list[ep_order] = i;
				ep_v2_list[ep_order] = j;
				ep_EventTime_list[ep_order] = get(boost::edge_weight_t(), input_graph, boost::edge(i, j, input_graph).first) / distribution_ratio; // halve the edge cost
				ep_ep2_order_list[ep_order] = ep_order + 1; // it points to the ep below
				node0.index = ep_order; // node index: ep order
				node0.priority_value = ep_EventTime_list[ep_order]; // priority: ep_EventTime
				handle_ep[ep_order] = C_ep_PQ[i].push(node0); // put this ep into cluster i
				ep_order++;
				// the second ep
				ep_v1_list[ep_order] = j;
				ep_v2_list[ep_order] = i;
				ep_EventTime_list[ep_order] = ep_EventTime_list[ep_order - 1] * (distribution_ratio - 1); // halve the edge cost
				ep_ep2_order_list[ep_order] = ep_order - 1; // it points to the ep above
				node0.index = ep_order; // node index: ep order
				node0.priority_value = ep_EventTime_list[ep_order]; // priority: ep_EventTime
				handle_ep[ep_order] = C_ep_PQ[j].push(node0); // put this ep into cluster j
				ep_order++;
			}
		});
		// for active cluster
		if (get(boost::vertex_name_t(), input_graph, i) > 0) {
			Active_C_num++; // the number of active clusters
			C_activity[i] = true; // this cluster is active
			C_event_time[i] = get(boost::vertex_name_t(), input_graph, i); // the event time is the node weight
																		   // push node into C_event_PQ
			node0.index = i; // node index: cluster order
			node0.priority_value = C_event_time[i]; // priority: node weight
			handle_Cevent[i] = C_event_PQ.push(node0); // into PQ
													   // all the ep for cluster i have been inserted into C_ep_PQ[i]; Note that, it's only true when i starts from 0 and j>i above
													   // push node into E_event_PQ
			node0.priority_value = C_ep_PQ[i].top().priority_value; // priority: the closest ep time
			handle_Eevent[i] = E_event_PQ.push(node0); // into PQ

													   //cout << "C_ep_PQ[i].size():" << C_ep_PQ[i].size() << endl;
													   //cout << "C_ep_PQ[i].top().priority_value:" << C_ep_PQ[i].top().priority_value << endl;
													   //cout << "node0.priority_value:" << node0.priority_value << endl;
													   //cout << "E_event_PQ.size():" << E_event_PQ.size() << endl;
													   //cout << "E_event_PQ.top().priority_value:" << E_event_PQ.top().priority_value << endl;
													   //getchar();
		}
	}


	// FGW growth starts!
	graph output_graph(N); // the output graph
	for (int i = 0; i < N; i++) {
		double new_weight = get(boost::vertex_name_t(), input_graph, i);
		boost::put(boost::vertex_name_t(), output_graph, i, new_weight); // input node weights
	}
	int C0;
	int C1;
	int C2;
	int ep1;
	int ep2;
	double Tc;
	double Te;
	double r;
	double lowerbound = 1e-7;  // d is not used in this coding

	auto begin_time = std::chrono::high_resolution_clock::now(); // start time

	while (Active_C_num > 1) // stop the loop when there is only one active cluster left
	{
		// find the closest event
		Tc = C_event_PQ.top().priority_value; // this cluster event time
		Te = E_event_PQ.top().priority_value; // this edge event time

		if (Tc >= Te) { // edge event
			C1 = E_event_PQ.top().index; // the cluster C1 for this edge event
			ep1 = C_ep_PQ[C1].top().index; // the top ep in C1
			C2 = V_locator[ep_v2_list[ep1]]; // the cluster C2 for this edge event

			if (C1 == C2) { // the inside ep is triggered
				C_ep_PQ[C1].pop(); // pop out the inside ep
								   // decrease E_event_PQ for the change of event_C1
				node0.index = C1;
				node0.priority_value = C_ep_PQ[C1].top().priority_value; // theoretically C_ep_PQ[event_C1] should not be empty
				E_event_PQ.decrease(handle_Eevent[C1], node0);
			}
			else { // the outside ep is triggered
				Global_time = Te;
				ep2 = ep_ep2_order_list[ep1];

				if (C_activity[C2] == true) { // C2 is active
					r = ep_EventTime_list[ep2] - Global_time; // the slack of the responsible edge

					if (r > lowerbound) { // r is big; d is not used in this coding
										  // change two ep event time
						ep_EventTime_list[ep1] = Global_time + r / 2;
						ep_EventTime_list[ep2] = Global_time + r / 2;
						// update C_ep_PQ in C1
						node0.index = ep1;
						node0.priority_value = ep_EventTime_list[ep1];
						C_ep_PQ[C1].decrease(handle_ep[ep1], node0);
						// update C_ep_PQ in C2
						node0.index = ep2;
						node0.priority_value = ep_EventTime_list[ep2];
						C_ep_PQ[C2].increase(handle_ep[ep2], node0);
						// update E_event_PQ for the change of C1
						node0.index = C1;
						node0.priority_value = C_ep_PQ[C1].top().priority_value;
						E_event_PQ.decrease(handle_Eevent[C1], node0);
						// update E_event_PQ for the change of C2
						if (C_ep_PQ[C2].top().index == ep2) {
							node0.index = C2;
							node0.priority_value = C_ep_PQ[C2].top().priority_value;
							E_event_PQ.increase(handle_Eevent[C2], node0);
						}
					}
					else { // r is small; merge event
						   // add edge with the original cost
						boost::add_edge(ep_v1_list[ep1], ep_v1_list[ep2],
							get(boost::edge_weight_t(), input_graph,
								boost::edge(ep_v1_list[ep1], ep_v1_list[ep2], input_graph).first), output_graph);
						// merge V_list of C2 into C1
						C_V_list[C1].insert(end(C_V_list[C1]), begin(C_V_list[C2]), end(C_V_list[C2]));
						//decrease V_locator
						for (int i = 0; i < C_V_list[C2].size(); i++) {
							V_locator[C_V_list[C2][i]] = C1;
						}
						// update event time of C1
						C_event_time[C1] = C_event_time[C1] + C_event_time[C2] - Global_time;
						// minus one active cluster
						C_activity[C2] = false;
						Active_C_num--;
						// merge two C_ep_PQ
						C_ep_PQ[C1].pop(); // pop out the responsible ep
						C_ep_PQ[C1].merge(C_ep_PQ[C2]);
						//fibonacci_heap<node>().swap(C_ep_PQ[C2]);
						// update C1 in C_event_time and E_event_time
						node0.index = C1;
						node0.priority_value = C_event_time[C1];
						C_event_PQ.decrease(handle_Cevent[C1], node0);
						node0.priority_value = C_ep_PQ[C1].top().priority_value;
						E_event_PQ.decrease(handle_Eevent[C1], node0);
						// remove C2 from C_event_time and E_event_time
						C_event_PQ.erase(handle_Cevent[C2]);
						E_event_PQ.erase(handle_Eevent[C2]);
					}
				}
				else { // C2 is inactive
					r = ep_EventTime_list[ep2] - C_event_time[C2]; // the slack of the responsible edge

					if (r > lowerbound) { // r is big; d is not used in this coding
										  // change two ep event time
						ep_EventTime_list[ep1] = Global_time + r;
						ep_EventTime_list[ep2] = C_event_time[C2];
						// update C_ep_PQ in C1
						node0.index = ep1;
						node0.priority_value = ep_EventTime_list[ep1];
						C_ep_PQ[C1].decrease(handle_ep[ep1], node0);
						// update C_ep_PQ in C2
						node0.index = ep2;
						node0.priority_value = ep_EventTime_list[ep2];
						C_ep_PQ[C2].increase(handle_ep[ep2], node0);
						// update E_event_PQ for the change of C1
						node0.index = C1;
						node0.priority_value = C_ep_PQ[C1].top().priority_value;
						E_event_PQ.decrease(handle_Eevent[C1], node0);
					}
					else { // r is small; merge event
						   // add edge
						boost::add_edge(ep_v1_list[ep1], ep_v1_list[ep2],
							get(boost::edge_weight_t(), input_graph,
								boost::edge(ep_v1_list[ep1], ep_v1_list[ep2], input_graph).first), output_graph);
						// merge V_list of C2 into C1
						C_V_list[C1].insert(end(C_V_list[C1]), begin(C_V_list[C2]), end(C_V_list[C2]));
						//decrease V_locator
						for (int i = 0; i < C_V_list[C2].size(); i++) {
							V_locator[C_V_list[C2][i]] = C1;
						}
						// merge two C_ep_PQ
						C_ep_PQ[C1].pop(); // pop out the responsible ep		   
						typename pairing_heap<node>::iterator begin = C_ep_PQ[C2].begin();
						typename pairing_heap<node>::iterator end = C_ep_PQ[C2].end();
						for (typename pairing_heap<node>::iterator it = begin; it != end; ++it)
						{
							node0 = *it;
							if (V_locator[ep_v2_list[node0.index]] != C1) { // only push outside nodes into C_ep_PQ[event_C1]; it's a little faster than not do that
								node0.priority_value = node0.priority_value + Global_time - C_event_time[C2]; // decrease priority values
								handle_ep[node0.index] = C_ep_PQ[C1].push(node0); // push; decrease handle
							}
						}
						// the code below is slower than that above
						//while (C_ep_PQ[C2].size() > 0) {
						//	node0 = C_ep_PQ[C2].top();
						//	C_ep_PQ[C2].pop();
						//	if (V_locator[ep_v2_list[node0.index]] != C1) { // only push outside nodes into C_ep_PQ[event_C1]; it's a little faster than not do that
						//		node0.priority_value = node0.priority_value + Global_time - C_event_time[C2]; // decrease priority values
						//		handle_ep[node0.index] = C_ep_PQ[C1].push(node0); // push; update handle
						//	}
						//}
						//// the code below is very slow
						//typename fibonacci_heap<node>::iterator begin = C_ep_PQ[C2].begin();
						//typename fibonacci_heap<node>::iterator end = C_ep_PQ[C2].end();
						//for (typename fibonacci_heap<node>::iterator it = begin; it != end; ++it)
						//{
						//	node0 = *it;
						//	node0.priority_value = node0.priority_value + Global_time - C_event_time[C2]; // update priority values
						//	C_ep_PQ[C2].decrease(handle_ep[node0.index], node0);
						//}
						//C_ep_PQ[C1].merge(C_ep_PQ[C2]);

						// decrease C1 in E_event_time
						node0.index = C1;
						node0.priority_value = C_ep_PQ[C1].top().priority_value;
						E_event_PQ.decrease(handle_Eevent[C1], node0);
					}
				}
			}
		}
		else { // cluster event
			Global_time = Tc; // decrease time
			C0 = C_event_PQ.top().index; // the cluster for this cluster event
			Active_C_num--; // minus one active cluster
			C_event_PQ.pop(); // remove the cluster from C_event_PQ
			E_event_PQ.erase(handle_Eevent[C0]); // remove the cluster from E_event_PQ
			C_activity[C0] = false; // deactivate it
		}
	}

	


	// remove disconnected parts
	std::vector<int> component(N); // vertex i is in component[i]; No.component from 0
	int cpn_num = connected_components(output_graph, &component[0]); // the number of component; decrease component
	int R_cpn = component[C_V_list[C_event_PQ.top().index][0]]; // it throw exception when TP=0 and C_event_PQ.size()=0
																//for (int i = 0; i < N; i++) {
																//	if (component[i] != R_cpn && in_degree(i, output_graph) > 0) { // disconnected vertex
																//		graph::out_edge_iterator eit, eend;
																//		tie(eit, eend) = boost::out_edges(i, output_graph); // adjacent_vertices of i
																//		for_each(eit, eend,
																//			[&output_graph, &i](graph::edge_descriptor it)
																//		{
																//			int j = boost::target(it, output_graph);
																//			if (j > i) {
																//				boost::remove_edge(i, j, output_graph);
																//			}
																//		});
																//	}
																//}
	for (int i = 0; i < N; i++) {
		if (component[i] != R_cpn && in_degree(i, output_graph) > 0) { // disconnected vertex
			clear_vertex(i, output_graph); // clear_vertex removes adjacent vertices, but not node weight
		}
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	double runningtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count(); // Nanosecond
	growth_time = runningtime / 1e6;

	return output_graph;

}
#pragma endregion FGW_V1  2018年4月28日16:11:26


#pragma region

graph FGW_eventsNum(graph input_graph, double& growth_time, double distribution_ratio, int& edge_event_num, int& cluster_event_num, int& total_event_num) {

	double Global_time = 0; // global time
	int Active_C_num = 0; // the number of active clusters

	int N = num_vertices(input_graph); // number of vertices
	int ep_num = num_edges(input_graph) * 2; // total number of edge parts
	int ep_order = 0;
	node node0;

	// Clusters: the number of clusters is always N
	vector<bool> C_activity(N); // activity value of each C; false means inactive; initial value is false
	vector<double> C_event_time(N); // the event time for each C
	vector<vector<int>> C_V_list(N); // record the vertices in each C
	vector<pairing_heap<node>> C_ep_PQ(N); // the PQ for edge parts in each C; node index: ep order in ep_list
	vector<int> V_locator(N); // the index of the maximal cluster containing the vertex
							  // edge parts: PQ and their handles
	vector<int> ep_v1_list(ep_num); // class may be slow, so I seperate the ep_list
	vector<int> ep_v2_list(ep_num);
	vector<double> ep_EventTime_list(ep_num);
	vector<int> ep_ep2_order_list(ep_num);
	vector<handle_t> handle_ep(ep_num); // store the handle for each edge part
										// the event PQ and their handles
	pairing_heap<node> C_event_PQ; // PQ storing the event time of the active clusters; node index: cluster order
	vector<handle_t> handle_Cevent(N);
	pairing_heap<node> E_event_PQ; // PQ storing the active clusters; node index: cluster order
	vector<handle_t> handle_Eevent(N);

	graph::out_edge_iterator eit, eend;

	// initialize the clusters
	for (int i = 0; i < N; i++)
	{
		C_V_list[i].insert(C_V_list[i].end(), i); // insert a vertex into the rear of C_V_list[i]
		V_locator[i] = i; // the maximal cluster containing vertex i
						  // add edge parts into C
		tie(eit, eend) = boost::out_edges(i, input_graph); // adjacent_vertices of i
		for_each(eit, eend,
			[&input_graph, &ep_v1_list, &ep_v2_list, &ep_ep2_order_list, &handle_ep, &C_ep_PQ, &node0, // the & above is the capture-list: the variables you can use inside
			&ep_order, &i, &ep_EventTime_list, &distribution_ratio](graph::edge_descriptor it) // for each adjacenct vertex boost::target(it, input_graph)
		{
			int j = boost::target(it, input_graph); // the adjacent vetex to i
			if (j > i) { // don't overcheck an edge
						 // the first ep
				ep_v1_list[ep_order] = i;
				ep_v2_list[ep_order] = j;
				ep_EventTime_list[ep_order] = get(boost::edge_weight_t(), input_graph, boost::edge(i, j, input_graph).first) / distribution_ratio; // halve the edge cost
				ep_ep2_order_list[ep_order] = ep_order + 1; // it points to the ep below
				node0.index = ep_order; // node index: ep order
				node0.priority_value = ep_EventTime_list[ep_order]; // priority: ep_EventTime
				handle_ep[ep_order] = C_ep_PQ[i].push(node0); // put this ep into cluster i
				ep_order++;
				// the second ep
				ep_v1_list[ep_order] = j;
				ep_v2_list[ep_order] = i;
				ep_EventTime_list[ep_order] = ep_EventTime_list[ep_order - 1] * (distribution_ratio - 1); // halve the edge cost
				ep_ep2_order_list[ep_order] = ep_order - 1; // it points to the ep above
				node0.index = ep_order; // node index: ep order
				node0.priority_value = ep_EventTime_list[ep_order]; // priority: ep_EventTime
				handle_ep[ep_order] = C_ep_PQ[j].push(node0); // put this ep into cluster j
				ep_order++;
			}
		});
		// for active cluster
		if (get(boost::vertex_name_t(), input_graph, i) > 0) {
			Active_C_num++; // the number of active clusters
			C_activity[i] = true; // this cluster is active
			C_event_time[i] = get(boost::vertex_name_t(), input_graph, i); // the event time is the node weight
																		   // push node into C_event_PQ
			node0.index = i; // node index: cluster order
			node0.priority_value = C_event_time[i]; // priority: node weight
			handle_Cevent[i] = C_event_PQ.push(node0); // into PQ
													   // all the ep for cluster i have been inserted into C_ep_PQ[i]; Note that, it's only true when i starts from 0 and j>i above
													   // push node into E_event_PQ
			node0.priority_value = C_ep_PQ[i].top().priority_value; // priority: the closest ep time
			handle_Eevent[i] = E_event_PQ.push(node0); // into PQ

													   //cout << "C_ep_PQ[i].size():" << C_ep_PQ[i].size() << endl;
													   //cout << "C_ep_PQ[i].top().priority_value:" << C_ep_PQ[i].top().priority_value << endl;
													   //cout << "node0.priority_value:" << node0.priority_value << endl;
													   //cout << "E_event_PQ.size():" << E_event_PQ.size() << endl;
													   //cout << "E_event_PQ.top().priority_value:" << E_event_PQ.top().priority_value << endl;
													   //getchar();
		}
	}


	// FGW growth starts!
	graph output_graph(N); // the output graph
	for (int i = 0; i < N; i++) {
		double new_weight = get(boost::vertex_name_t(), input_graph, i);
		boost::put(boost::vertex_name_t(), output_graph, i, new_weight); // input node weights
	}
	int C0;
	int C1;
	int C2;
	int ep1;
	int ep2;
	double Tc;
	double Te;
	double r;
	double lowerbound = 1e-7;  // d is not used in this coding

	auto begin_time = std::chrono::high_resolution_clock::now(); // start time

	edge_event_num = 0;
	cluster_event_num = 0;
	total_event_num = 0;

	while (Active_C_num > 1) // stop the loop when there is only one active cluster left
	{
		// find the closest event
		Tc = C_event_PQ.top().priority_value; // this cluster event time
		Te = E_event_PQ.top().priority_value; // this edge event time

		total_event_num++;

		if (Tc >= Te) { // edge event
			edge_event_num++;
			C1 = E_event_PQ.top().index; // the cluster C1 for this edge event
			ep1 = C_ep_PQ[C1].top().index; // the top ep in C1
			C2 = V_locator[ep_v2_list[ep1]]; // the cluster C2 for this edge event

			if (C1 == C2) { // the inside ep is triggered
				C_ep_PQ[C1].pop(); // pop out the inside ep
								   // decrease E_event_PQ for the change of event_C1
				node0.index = C1;
				node0.priority_value = C_ep_PQ[C1].top().priority_value; // theoretically C_ep_PQ[event_C1] should not be empty
				E_event_PQ.decrease(handle_Eevent[C1], node0);
			}
			else { // the outside ep is triggered
				Global_time = Te;
				ep2 = ep_ep2_order_list[ep1];

				if (C_activity[C2] == true) { // C2 is active
					r = ep_EventTime_list[ep2] - Global_time; // the slack of the responsible edge

					if (r > lowerbound) { // r is big; d is not used in this coding
										  // change two ep event time
						ep_EventTime_list[ep1] = Global_time + r / 2;
						ep_EventTime_list[ep2] = Global_time + r / 2;
						// update C_ep_PQ in C1
						node0.index = ep1;
						node0.priority_value = ep_EventTime_list[ep1];
						C_ep_PQ[C1].decrease(handle_ep[ep1], node0);
						// update C_ep_PQ in C2
						node0.index = ep2;
						node0.priority_value = ep_EventTime_list[ep2];
						C_ep_PQ[C2].increase(handle_ep[ep2], node0);
						// update E_event_PQ for the change of C1
						node0.index = C1;
						node0.priority_value = C_ep_PQ[C1].top().priority_value;
						E_event_PQ.decrease(handle_Eevent[C1], node0);
						// update E_event_PQ for the change of C2
						if (C_ep_PQ[C2].top().index == ep2) {
							node0.index = C2;
							node0.priority_value = C_ep_PQ[C2].top().priority_value;
							E_event_PQ.increase(handle_Eevent[C2], node0);
						}
					}
					else { // r is small; merge event
						   // add edge with the original cost
						boost::add_edge(ep_v1_list[ep1], ep_v1_list[ep2],
							get(boost::edge_weight_t(), input_graph,
								boost::edge(ep_v1_list[ep1], ep_v1_list[ep2], input_graph).first), output_graph);
						// merge V_list of C2 into C1
						C_V_list[C1].insert(end(C_V_list[C1]), begin(C_V_list[C2]), end(C_V_list[C2]));
						//decrease V_locator
						for (int i = 0; i < C_V_list[C2].size(); i++) {
							V_locator[C_V_list[C2][i]] = C1;
						}
						// update event time of C1
						C_event_time[C1] = C_event_time[C1] + C_event_time[C2] - Global_time;
						// minus one active cluster
						Active_C_num--;
						// merge two C_ep_PQ
						C_ep_PQ[C1].pop(); // pop out the responsible ep
						C_ep_PQ[C1].merge(C_ep_PQ[C2]);
						//fibonacci_heap<node>().swap(C_ep_PQ[C2]);
						// update C1 in C_event_time and E_event_time
						node0.index = C1;
						node0.priority_value = C_event_time[C1];
						C_event_PQ.decrease(handle_Cevent[C1], node0);
						node0.priority_value = C_ep_PQ[C1].top().priority_value;
						E_event_PQ.decrease(handle_Eevent[C1], node0);
						// remove C2 from C_event_time and E_event_time
						C_event_PQ.erase(handle_Cevent[C2]);
						E_event_PQ.erase(handle_Eevent[C2]);
					}
				}
				else { // C2 is inactive
					r = ep_EventTime_list[ep2] - C_event_time[C2]; // the slack of the responsible edge

					if (r > lowerbound) { // r is big; d is not used in this coding
										  // change two ep event time
						ep_EventTime_list[ep1] = Global_time + r;
						ep_EventTime_list[ep2] = C_event_time[C2];
						// update C_ep_PQ in C1
						node0.index = ep1;
						node0.priority_value = ep_EventTime_list[ep1];
						C_ep_PQ[C1].decrease(handle_ep[ep1], node0);
						// update C_ep_PQ in C2
						node0.index = ep2;
						node0.priority_value = ep_EventTime_list[ep2];
						C_ep_PQ[C2].increase(handle_ep[ep2], node0);
						// update E_event_PQ for the change of C1
						node0.index = C1;
						node0.priority_value = C_ep_PQ[C1].top().priority_value;
						E_event_PQ.decrease(handle_Eevent[C1], node0);
					}
					else { // r is small; merge event
						   // add edge
						boost::add_edge(ep_v1_list[ep1], ep_v1_list[ep2],
							get(boost::edge_weight_t(), input_graph,
								boost::edge(ep_v1_list[ep1], ep_v1_list[ep2], input_graph).first), output_graph);
						// merge V_list of C2 into C1
						C_V_list[C1].insert(end(C_V_list[C1]), begin(C_V_list[C2]), end(C_V_list[C2]));
						//decrease V_locator
						for (int i = 0; i < C_V_list[C2].size(); i++) {
							V_locator[C_V_list[C2][i]] = C1;
						}
						// merge two C_ep_PQ
						C_ep_PQ[C1].pop(); // pop out the responsible ep		   
						typename pairing_heap<node>::iterator begin = C_ep_PQ[C2].begin();
						typename pairing_heap<node>::iterator end = C_ep_PQ[C2].end();
						for (typename pairing_heap<node>::iterator it = begin; it != end; ++it)
						{
							node0 = *it;
							if (V_locator[ep_v2_list[node0.index]] != C1) { // only push outside nodes into C_ep_PQ[event_C1]; it's a little faster than not do that
								node0.priority_value = node0.priority_value + Global_time - C_event_time[C2]; // decrease priority values
								handle_ep[node0.index] = C_ep_PQ[C1].push(node0); // push; decrease handle
							}
						}
						// the code below is slower than that above
						//while (C_ep_PQ[C2].size() > 0) {
						//	node0 = C_ep_PQ[C2].top();
						//	C_ep_PQ[C2].pop();
						//	if (V_locator[ep_v2_list[node0.index]] != C1) { // only push outside nodes into C_ep_PQ[event_C1]; it's a little faster than not do that
						//		node0.priority_value = node0.priority_value + Global_time - C_event_time[C2]; // decrease priority values
						//		handle_ep[node0.index] = C_ep_PQ[C1].push(node0); // push; update handle
						//	}
						//}
						//// the code below is very slow
						//typename fibonacci_heap<node>::iterator begin = C_ep_PQ[C2].begin();
						//typename fibonacci_heap<node>::iterator end = C_ep_PQ[C2].end();
						//for (typename fibonacci_heap<node>::iterator it = begin; it != end; ++it)
						//{
						//	node0 = *it;
						//	node0.priority_value = node0.priority_value + Global_time - C_event_time[C2]; // update priority values
						//	C_ep_PQ[C2].decrease(handle_ep[node0.index], node0);
						//}
						//C_ep_PQ[C1].merge(C_ep_PQ[C2]);

						// decrease C1 in E_event_time
						node0.index = C1;
						node0.priority_value = C_ep_PQ[C1].top().priority_value;
						E_event_PQ.decrease(handle_Eevent[C1], node0);
					}
				}
			}
		}
		else { // cluster event
			cluster_event_num++;
			Global_time = Tc; // decrease time
			C0 = C_event_PQ.top().index; // the cluster for this cluster event
			Active_C_num--; // minus one active cluster
			C_event_PQ.pop(); // remove the cluster from C_event_PQ
			E_event_PQ.erase(handle_Eevent[C0]); // remove the cluster from E_event_PQ
			C_activity[C0] = false; // deactivate it
		}
	}

	// remove disconnected parts
	std::vector<int> component(N); // vertex i is in component[i]; No.component from 0
	int cpn_num = connected_components(output_graph, &component[0]); // the number of component; decrease component
	int R_cpn = component[C_V_list[C_event_PQ.top().index][0]]; // it throw exception when TP=0 and C_event_PQ.size()=0
																//for (int i = 0; i < N; i++) {
																//	if (component[i] != R_cpn && in_degree(i, output_graph) > 0) { // disconnected vertex
																//		graph::out_edge_iterator eit, eend;
																//		tie(eit, eend) = boost::out_edges(i, output_graph); // adjacent_vertices of i
																//		for_each(eit, eend,
																//			[&output_graph, &i](graph::edge_descriptor it)
																//		{
																//			int j = boost::target(it, output_graph);
																//			if (j > i) {
																//				boost::remove_edge(i, j, output_graph);
																//			}
																//		});
																//	}
																//}
	for (int i = 0; i < N; i++) {
		if (component[i] != R_cpn && in_degree(i, output_graph) > 0) { // disconnected vertex
			clear_vertex(i, output_graph); // clear_vertex removes adjacent vertices, but not node weight
		}
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	double runningtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count(); // Nanosecond
	growth_time = runningtime / 1e6;

	return output_graph;

}
#pragma endregion FGW_eventsNum


#pragma region
graph GPrA(graph input_graph, double& GPrA_time) {

	int N = num_vertices(input_graph); // number of vertices
	vector<double> nw1(N); // the nw value for finding R, it will be decreased after the check
	vector<double> nw2(N); // the nw value for pruning
	for (int i = 0; i < N; i++) {
		nw1[i] = get(boost::vertex_name_t(), input_graph, i);
		nw2[i] = get(boost::vertex_name_t(), input_graph, i);
	}
	vector<bool> pcheck1(N); // true means it has been checked 
	int num_check1 = 0; // the number of checked vertices to find R
	vector<bool> pcheck2(N); // true means it has been checked 
	int num_check2 = 0; // the number of checked vertices for pruning
	vector<int> pdegree1(N); // the processing degree to find R
	vector<int> pdegree2(N); // the processing degree for pruning
	vector<int> leaf1; // the leaves for finding R
	vector<int> leaf2; // the leaves for pruning

	for (int i = 0; i < N; i++) {
		pdegree1[i] = in_degree(i, input_graph); // decrease pdegree
		pdegree2[i] = pdegree1[i];
		if (pdegree1[i] == 0) {
			pcheck1[i] = true; // check disconnected vertices
			num_check1++;
			pcheck2[i] = true;
			num_check2++;
		}
		else if (pdegree1[i] == 1) {
			leaf1.insert(leaf1.end(), i);
			leaf2.insert(leaf2.end(), i);
		}
	}

	graph::out_edge_iterator eit, eend;
	AdjacencyIterator ai, a_end;
	int leaf_num1 = N - num_check1 - 1; // the number of leaves you need to process
	int leaf_num2 = N - num_check1 - 1; // the number of leaves you need to process

	auto begin_time = std::chrono::high_resolution_clock::now(); // start time

																 //this version is similar to the version below
	int k = 0;
	while (k < leaf_num1) {
		int i = leaf1[k];
		k++;
		tie(eit, eend) = boost::out_edges(i, input_graph); // adjacent_vertices of i
		for_each(eit, eend,
			[&input_graph, &pcheck1, &i, &nw1, &pdegree1, &num_check1, &leaf1](graph::edge_descriptor it)
		{
			int j = boost::target(it, input_graph);
			if (pcheck1[j] == false) {
				double cost = get(boost::edge_weight_t(), input_graph, boost::edge(i, j, input_graph).first);
				if (cost < nw1[i]) {
					nw1[j] = nw1[j] + nw1[i] - cost; // decrease nw1[j]
				}
				pcheck1[i] = true; // i has been checked, there is no need to delete it in this phase
				pdegree1[j]--;// decrease pdegree[j]
				if (pdegree1[j] == 1) {
					leaf1.insert(leaf1.end(), j); // it's fine to insert in the end, but not in the biginning
				}
				// break; // how to break a for_each???
			}
		});
	}

	//// find the Root, which is the mark of the optimal prunning result
	//while (num_check1 < N - 1) { 
	//	// there will be a single vertex left unchecked (but its nw value will be decreased)
	//	//// the version below is slower
	//	//while (leaf1.size() > 0) {
	//	//	int i = leaf1[0];
	//	//	leaf1.erase(leaf1.begin());
	//	//	tie(eit, eend) = boost::out_edges(i, input_graph); // adjacent_vertices of i
	//	//	for_each(eit, eend,
	//	//		[&input_graph, &pcheck1, &i, &nw1, &pdegree1, &num_check1, &leaf1](graph::edge_descriptor it)
	//	//	{
	//	//		int j = boost::target(it, input_graph);
	//	//		if (pcheck1[j] == false) {
	//	//			double cost = get(boost::edge_weight_t(), input_graph, boost::edge(i, j, input_graph).first);
	//	//			if (cost < nw1[i]) {
	//	//				nw1[j] = nw1[j] + nw1[i] - cost; // decrease nw1[j]
	//	//			}
	//	//			pcheck1[i] = true; // i has been checked, there is no need to delete it in this phase
	//	//			num_check1++;
	//	//			pdegree1[j]--;// decrease pdegree[j]
	//	//			if (pdegree1[j] == 1) {
	//	//				leaf1.insert(leaf1.end(), j); // it's fine to insert in the end, but not in the biginning; note for (int k = 0; k < leaf1.size(); k++)
	//	//			}
	//	//			// break; // how to break a for_each???
	//	//		}
	//	//	});
	//	//}
	//	//// the version below is fast
	//	for (int k = 0; k < leaf1.size(); k++)
	//	{
	//		int i = leaf1[k];
	//		if (pdegree1[i] == 1) {
	//			tie(eit, eend) = boost::out_edges(i, input_graph); // adjacent_vertices of i
	//			for_each(eit, eend,
	//				[&input_graph, &pcheck1, &i, &nw1, &pdegree1, &num_check1, &leaf1, &k](graph::edge_descriptor it)
	//			{
	//				int j = boost::target(it, input_graph);
	//				if (pcheck1[j] == false) {
	//					double cost = get(boost::edge_weight_t(), input_graph, boost::edge(i, j, input_graph).first);
	//					if (cost < nw1[i]) {
	//						nw1[j] = nw1[j] + nw1[i] - cost; // decrease nw1[j]
	//					}
	//					pcheck1[i] = true; // i has been checked, there is no need to delete it in this phase
	//					num_check1++;
	//					pdegree1[i] = 0; // it's not the leaf any more
	//					pdegree1[j]--; // decrease pdegree[j]
	//					if (pdegree1[j] == 1) {
	//						leaf1.insert(leaf1.end(), j); // it's fine to insert in the end, but not in the biginning; note for (int k = 0; k < leaf1.size(); k++)
	//					}
	//					// break; // how to break a for_each???
	//				}
	//			});
	//		}
	//		//// the version below is slower than that above
	//		//boost::tie(ai, a_end) = boost::adjacent_vertices(i, input_graph);
	//		//for (; ai != a_end; ai++) {
	//		//	int j = *ai;
	//		//	if (pcheck1[j] == false) {
	//		//		double cost = get(boost::edge_weight_t(), input_graph, boost::edge(i, j, input_graph).first);
	//		//		if (cost < nw1[i]) {
	//		//			nw1[j] = nw1[j] + nw1[i] - cost; // decrease nw1[j]
	//		//		}
	//		//		pcheck1[i] = true; // i has been checked, there is no need to delete it in this phase
	//		//		num_check1++;
	//		//		pdegree1[i] = 0; // it's not the leaf any more
	//		//		pdegree1[j]--;// decrease pdegree[j]
	//		//if (pdegree1[j] == 1) {
	//		//	leaf1.insert(leaf1.end(), j);
	//		//}
	//		//		break; // how to break a for_each???
	//		//	}
	//		//}
	//	}
	//}

	// R is the vertex with the biggest nw
	int R = 0;
	double max = nw1[0];
	for (int i = 1; i < N; i++) {
		if (nw1[i] > max) {
			max = nw1[i];
			R = i;
		}
	}

	// Strong pruning tree
	graph output_graph = input_graph; // the output graph

									  //this version is similar to the version below
	k = 0;
	while (k < leaf_num2 + 1) { // since R is ignored, it must be leaf_num2+1
		int i = leaf2[k];
		k++;
		if (i != R) {
			tie(eit, eend) = boost::out_edges(i, output_graph); // adjacent_vertices of i
			for_each(eit, eend,
				[&output_graph, &pcheck2, &i, &nw2, &pdegree2, &num_check2, &leaf2, &k](graph::edge_descriptor it)
			{
				int j = boost::target(it, output_graph);
				if (pcheck2[j] == false) {
					double cost = get(boost::edge_weight_t(), output_graph, boost::edge(i, j, output_graph).first);
					if (cost < nw2[i]) {
						nw2[j] = nw2[j] + nw2[i] - cost; // decrease nw2[j]
					}
					else {
						boost::remove_edge(i, j, output_graph); // remove edge(i,j)	
					}
					pcheck2[i] = true; // i has been checked
					pdegree2[j]--;// decrease pdegree[j]
					if (pdegree2[j] == 1) {
						leaf2.insert(leaf2.end(), j);
					}
					// break; // how to break a for_each???
				}
			});
		}
	}

	//while (num_check2 < N - 1) {
	//	for (int k = 0; k < leaf2.size(); k++)
	//	{
	//		int i = leaf2[k];
	//		if (pdegree2[i] == 1 && i != R) {
	//			tie(eit, eend) = boost::out_edges(i, output_graph); // adjacent_vertices of i
	//			for_each(eit, eend,
	//				[&output_graph, &pcheck2, &i, &nw2, &pdegree2, &num_check2, &leaf2, &k](graph::edge_descriptor it)
	//			{
	//				int j = boost::target(it, output_graph);
	//				if (pcheck2[j] == false) {
	//					double cost = get(boost::edge_weight_t(), output_graph, boost::edge(i, j, output_graph).first);
	//					if (cost < nw2[i]) {
	//						nw2[j] = nw2[j] + nw2[i] - cost; // decrease nw2[j]
	//					}
	//					else {
	//						boost::remove_edge(i, j, output_graph); // remove edge(i,j)	
	//					}
	//					pcheck2[i] = true; // i has been checked
	//					num_check2++;
	//					pdegree2[i] = 0; // it's not the leaf any more
	//					pdegree2[j]--;// decrease pdegree[j]
	//					if (pdegree2[j] == 1) {
	//						leaf2.insert(leaf2.end(), j);
	//					}
	//					// break; // how to break a for_each???
	//				}
	//			});
	//			// the version below is slower than that above
	//			//boost::tie(ai, a_end) = boost::adjacent_vertices(i, input_graph);
	//			//for (; ai != a_end; ai++) {
	//			//	int j = *ai;
	//			//	if (pcheck2[j] == false) {
	//			//		double cost = get(boost::edge_weight_t(), output_graph, boost::edge(i, j, output_graph).first);
	//			//		if (cost < nw2[i]) {
	//			//			nw2[j] = nw2[j] + nw2[i] - cost; // decrease nw2[j]
	//			//		}
	//			//		else {
	//			//			boost::remove_edge(i, j, output_graph); // remove edge(i,j)
	//			//			//// check
	//			//			//std::cout << "output_graph net_cost: " << net_cost(output_graph) << endl; // this line causes errors; becuase edge_descriptor? why?
	//			//		}
	//			//		pcheck2[i] = true; // i has been checked
	//			//		num_check2++;
	//			//		pdegree2[i] = 0; // it's not the leaf any more
	//			//		pdegree2[j]--;// decrease pdegree[j]
	//			//if (pdegree2[j] == 1) {
	//			//	leaf2.insert(leaf2.end(), j);
	//			//}
	//			//		break; 
	//			//	}
	//			//}
	//		}
	//	}
	//}

	// deleted disconnected parts
	std::vector<int> component(N); // vertex i is in component[i]; No.component from 0
	int cpn_num = connected_components(output_graph, &component[0]); // the number of component; decrease component
	for (int i = 0; i < N; i++) {
		if (component[i] != component[R]) { // disconnected vertex
			clear_vertex(i, output_graph); // clear_vertex removes adjacent vertices, but not node weight
		}
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	double runningtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count(); // Nanosecond
	GPrA_time = runningtime / 1e6;

	return output_graph;

}
#pragma endregion  GPrA  2016年11月26日18:06


#pragma region
graph copy_graph(graph input_graph) {

	graph output_graph = input_graph; // this input_graph is just a copy of the outside input_graph

	return output_graph;

}
#pragma endregion copy_graph 2017年11月22日13:05:11


#pragma region
std::vector<int> copy_vector_int(std::vector<int> input_vector) {

	std::vector<int> output_vector = input_vector;

	return output_vector;

}
#pragma endregion copy_vector_int 2018年2月10日13:25:57


#pragma region
std::vector<bool> copy_vector_bool(std::vector<bool> input_vector) {

	std::vector<bool> output_vector = input_vector;

	return output_vector;

}
#pragma endregion copy_vector_bool


#pragma region
std::vector<double> copy_vector_double(std::vector<double> input_vector) {

	std::vector<double> output_vector = input_vector;

	return output_vector;

}
#pragma endregion copy_vector_double


#pragma region
graph SPA(graph input_graph, int R) {

	int N = num_vertices(input_graph); // number of vertices
	vector<double> nw2(N); // the nw value for pruning
	for (int i = 0; i < N; i++) {
		nw2[i] = get(boost::vertex_name_t(), input_graph, i);
	}
	vector<bool> pcheck2(N); // true means it has been checked 
	int num_check2 = 0; // the number of checked vertices for pruning
	vector<int> pdegree2(N); // the processing degree for pruning
	vector<int> leaf2; // the leaves for pruning

	for (int i = 0; i < N; i++) {
		pdegree2[i] = in_degree(i, input_graph); // decrease pdegree
		if (pdegree2[i] == 0) {
			pcheck2[i] = true;
			num_check2++;
		}
		else if (pdegree2[i] == 1) {
			leaf2.insert(leaf2.end(), i);
		}
	}

	graph::out_edge_iterator eit, eend;
	AdjacencyIterator ai, a_end;
	int leaf_num2 = N - num_check2 - 1; // the number of leaves you need to process


	// Strong pruning tree
	graph output_graph = input_graph; // the output graph
	int k = 0;
	while (k < leaf_num2 + 1) { // since R is ignored, it must be leaf_num2+1
		int i = leaf2[k];
		k++;
		if (i != R) {
			tie(eit, eend) = boost::out_edges(i, output_graph); // adjacent_vertices of i
			for_each(eit, eend,
				[&output_graph, &pcheck2, &i, &nw2, &pdegree2, &leaf2, &k](graph::edge_descriptor it)
			{
				int j = boost::target(it, output_graph);
				if (pcheck2[j] == false) {
					double cost = get(boost::edge_weight_t(), output_graph, boost::edge(i, j, output_graph).first);
					if (cost < nw2[i]) {
						nw2[j] = nw2[j] + nw2[i] - cost; // decrease nw2[j]
					}
					else {
						boost::remove_edge(i, j, output_graph); // remove edge(i,j)	
					}
					pcheck2[i] = true; // i has been checked
					pdegree2[j]--;// decrease pdegree[j]
					if (pdegree2[j] == 1) {
						leaf2.insert(leaf2.end(), j);
					}
					// break; // how to break a for_each???
				}
			});
		}
	}

	// deleted disconnected parts
	std::vector<int> component(N); // vertex i is in component[i]; No.component from 0
	int cpn_num = connected_components(output_graph, &component[0]); // the number of component; decrease component
	for (int i = 0; i < N; i++) {
		if (component[i] != component[R]) { // disconnected vertex
			clear_vertex(i, output_graph); // clear_vertex removes adjacent vertices, but not node weight
		}
	}


	return output_graph;

}
#pragma endregion  SPA


#pragma region
graph SPA_random_R(graph input_graph, double& SPA_random_R_time) {

	int N = num_vertices(input_graph); // number of vertices
	vector<double> nw2(N); // the nw value for pruning
	for (int i = 0; i < N; i++) {
		nw2[i] = get(boost::vertex_name_t(), input_graph, i);
	}
	vector<bool> pcheck2(N); // true means it has been checked 
	int num_check2 = 0; // the number of checked vertices for pruning
	vector<int> pdegree2(N); // the processing degree for pruning
	vector<int> leaf2; // the leaves for pruning

	for (int i = 0; i < N; i++) {
		pdegree2[i] = in_degree(i, input_graph); // decrease pdegree
		if (pdegree2[i] == 0) {
			pcheck2[i] = true;
			num_check2++;
		}
		else if (pdegree2[i] == 1) {
			leaf2.insert(leaf2.end(), i);
		}
	}

	graph::out_edge_iterator eit, eend;
	AdjacencyIterator ai, a_end;
	int leaf_num2 = N - num_check2 - 1; // the number of leaves you need to process

	auto begin_time = std::chrono::high_resolution_clock::now(); // start time

	// included_V
	vector<int> included_V;
	for (int i = 0; i < N; i++) {
		if (in_degree(i, input_graph) > 0) { // we assume that input_graph is connected
			included_V.insert(included_V.end(), i);
		}
	}

	int random = rand() % included_V.size();
	int R = included_V[random]; // random R


	// Strong pruning tree
	graph output_graph = input_graph; // the output graph
	int k = 0;
	while (k < leaf_num2 + 1) { // since R is ignored, it must be leaf_num2+1
		int i = leaf2[k];
		k++;
		if (i != R) {
			tie(eit, eend) = boost::out_edges(i, output_graph); // adjacent_vertices of i
			for_each(eit, eend,
				[&output_graph, &pcheck2, &i, &nw2, &pdegree2, &leaf2, &k](graph::edge_descriptor it)
			{
				int j = boost::target(it, output_graph);
				if (pcheck2[j] == false) {
					double cost = get(boost::edge_weight_t(), output_graph, boost::edge(i, j, output_graph).first);
					if (cost < nw2[i]) {
						nw2[j] = nw2[j] + nw2[i] - cost; // decrease nw2[j]
					}
					else {
						boost::remove_edge(i, j, output_graph); // remove edge(i,j)	
					}
					pcheck2[i] = true; // i has been checked
					pdegree2[j]--;// decrease pdegree[j]
					if (pdegree2[j] == 1) {
						leaf2.insert(leaf2.end(), j);
					}
					// break; // how to break a for_each???
				}
			});
		}
	}

	

	// deleted disconnected parts
	std::vector<int> component(N); // vertex i is in component[i]; No.component from 0
	int cpn_num = connected_components(output_graph, &component[0]); // the number of component; decrease component
	for (int i = 0; i < N; i++) {
		if (component[i] != component[R]) { // disconnected vertex
			clear_vertex(i, output_graph); // clear_vertex removes adjacent vertices, but not node weight
		}
	}

	
	auto end_time = std::chrono::high_resolution_clock::now();
	double runningtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count(); // Nanosecond
	SPA_random_R_time = runningtime / 1e6;
	

	return output_graph;

}
#pragma endregion  SPA_random_R


#pragma region
graph random__multiR_SPA(graph input_graph, double& random__multiR_SPA_time, int random_R_num) {

	int N = num_vertices(input_graph); // number of vertices

	// included_V
	vector<int> included_V;
	for (int i = 0; i < N; i++) {
		if (in_degree(i, input_graph) > 0) { // we assume that input_graph is connected
			included_V.insert(included_V.end(), i);
		}
	}

	//cout << included_V.size() << endl;

	graph best_graph;
	double best_cost = 1e8;

	auto begin_time = std::chrono::high_resolution_clock::now(); // start time

	int times = 0;
	while (times < random_R_num) { // select R random_R_num times
		times++;

		int random = rand() % included_V.size();
		//cout << random << endl;
		int R = included_V[random]; // random R

		graph output_graph = SPA(input_graph, R);
		double prune_cost = net_cost(output_graph);

		if (prune_cost < best_cost) {
			best_cost = prune_cost;
			best_graph = copy_graph(output_graph);
		}

	}


	auto end_time = std::chrono::high_resolution_clock::now();
	double runningtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count(); // Nanosecond
	random__multiR_SPA_time = runningtime / 1e6;

	return best_graph;

}
#pragma endregion  random__multiR_SPA


#pragma region

double GW_LB(graph input_graph) {

	//the GW_LB is derived from Equation 1 in "A Fast, Adaptive Variant of the Goemans-Williamson Scheme for the Prize-Collecting Steiner Tree Problem"
	// GW_LB=1/2*(included_cost+2*missed_prize)

	double included_cost = 0;
	double missed_prize = 0;
	int N = num_vertices(input_graph); // number of vertices
	graph::out_edge_iterator eit1, eend1;

	if (num_edges(input_graph) == 0) {
		double sum_node_weight = get(boost::vertex_name_t(), input_graph, 0);
		double biggest_node_weight = get(boost::vertex_name_t(), input_graph, 0); // initialize as node weight 0
		for (int i = 1; i < N; i++) {
			double x = get(boost::vertex_name_t(), input_graph, i);
			sum_node_weight = sum_node_weight + x;
			if (biggest_node_weight < x) {
				biggest_node_weight = x; // find the maximal node weight
			}
		}
		missed_prize = sum_node_weight - biggest_node_weight;
	}
	else {
		for (int i = 0; i < N; i++) {
			if (in_degree(i, input_graph) > 0) { // the included vertex
				tie(eit1, eend1) = boost::out_edges(i, input_graph); // adjacent_vertices of i
				for_each(eit1, eend1,
					[&input_graph, &i, &included_cost](graph::edge_descriptor it1)
				{
					int j = boost::target(it1, input_graph); // the adjacent vetex to i
					if (j > i) { // don't overcount an edge
						included_cost = included_cost + get(boost::edge_weight_t(), input_graph, boost::edge(i, j, input_graph).first); // the edge cost
					}
				});
			}
			else {
				if (get(boost::vertex_name_t(), input_graph, i) > 0) {
					missed_prize = missed_prize + get(boost::vertex_name_t(), input_graph, i);
				}
			}
		}
	}

	double GW_lb = (included_cost + 2 * missed_prize) / 2;
	return GW_lb;
}

#pragma endregion GW_LB


#pragma region
void save_data_FGW(string instance_name, graph result_graph, double net_cost, double running_time, double distribution_ratio) {

	string save_name = "Result_FGW1_" + instance_name; // save_name
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	outputFile.open(save_name + ".stp"); // stp file

	outputFile << "33D32945 STP File, STP Format Version 1.0" << endl;
	outputFile << endl;

	// comments
	outputFile << "SECTION Comments" << endl;
	outputFile << "Name \"" << save_name << "\"" << endl;
	outputFile << "Net_cost " << net_cost << endl;
	outputFile << "Distribution_ratio " << distribution_ratio << endl;
	outputFile << "Running_time " << running_time << "ms" << endl;
	outputFile << "Creator \"Yahui Sun\"" << endl;
	outputFile << "Problem \"Prize - Collecting Steiner Problem in Graphs\"" << endl;
	outputFile << "END" << endl;
	outputFile << endl;

	// graph
	outputFile << "SECTION Graph" << endl;
	outputFile << "Nodes " << num_vertices(result_graph) << endl;
	outputFile << "Edges " << num_edges(result_graph) << endl;
	graph::out_edge_iterator eit, eend;
	for (int i = 0; i < num_vertices(result_graph); i++) {
		tie(eit, eend) = boost::out_edges(i, result_graph); // adjacent_vertices of 2
		for_each(eit, eend,
			[&result_graph, &i, &outputFile](graph::edge_descriptor it)
		{
			int j = boost::target(it, result_graph);
			if (i < j) {
				outputFile << "E " << i + 1 << " " << j + 1 << " " << get(boost::edge_weight_t(), result_graph, boost::edge(i, j, result_graph).first) << endl;
			}
		});
	}
	outputFile << "END" << endl;
	outputFile << endl;

	// TP
	outputFile << "SECTION Non-Compulsory Terminals" << endl;
	int p = 0;
	for (int i = 0; i < num_vertices(result_graph); i++) {
		if (get(boost::vertex_name_t(), result_graph, i) > 0) {
			p++;
		}
	}
	outputFile << "Terminals " << p << endl;
	for (int i = 0; i < num_vertices(result_graph); i++) {
		if (get(boost::vertex_name_t(), result_graph, i) > 0) {
			outputFile << "TP " << i + 1 << " " << get(boost::vertex_name_t(), result_graph, i) << endl;
		}
	}
	outputFile << "END" << endl;
	outputFile << endl;

	outputFile << "EOF" << endl;

}
#pragma endregion save_data_FGW  2016年11月26日18:36


#pragma region
void solve_hand_FGW_show_randD_effects() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 10000; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;

	for (int name = 48; name < 50; name++) {

		// instance_name
		if (name < 10) {
			instance = "handbd";
			instance_name = instance + to_string(0) + to_string(name);
		}
		else if (name < 15) {
			instance = "handbd";
			instance_name = instance + to_string(name);
		}
		else if (name < 24) {
			instance = "handbi";
			instance_name = instance + to_string(0) + to_string(name - 14);
		}
		else if (name < 29) {
			instance = "handbi";
			instance_name = instance + to_string(name - 14);
		}
		else if (name < 38) {
			instance = "handsd";
			instance_name = instance + to_string(0) + to_string(name - 28);
		}
		else if (name == 38) {
			instance = "handsd";
			instance_name = instance + to_string(name - 28);
		}
		else if (name < 48) {
			instance = "handsi";
			instance_name = instance + to_string(0) + to_string(name - 38);
		}
		else if (name == 48) {
			instance = "handsi";
			instance_name = instance + to_string(name - 38);
		}

		input_graph = read_data_with_terminals(instance_name + ".stp");

		outputFile << instance_name << endl;

		std::vector<pair<double, int>> solution_and_times;

		double best_solu = 1e8;
		double best_ratio;

		for (int i = 0; i < times; i++) {

			distribution_ratio = (double) (10000 + (rand() % (1000000 - 10000 + 1))) / 10000; // random from 1 to 100

			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
			//cout << "Growth Time= " << growth_time << "ms" << endl;
			// pruning Growth_graph
			GPrA_graph = GPrA(Growth_graph, GPrA_time);
			Solu_net_cost = net_cost(GPrA_graph);

			if (Solu_net_cost < best_solu) {
				best_solu = Solu_net_cost;
				best_ratio = distribution_ratio;
			}
			cout << "times: " << times << " Solu_net_cost: " << Solu_net_cost 
				<< " distribution_ratio: " << distribution_ratio << 
				" best_solu: " << best_solu << " best_ratio: " << best_ratio << endl;

			bool found_before = false;
			for (int j = 0; j < solution_and_times.size(); j++) {
				if (solution_and_times[j].first == Solu_net_cost) {
					solution_and_times[j].second++;
					found_before = true;
					break;
				}
			}
			if (found_before == false) {
				pair<double, int> new_solution = { Solu_net_cost ,1 };
				solution_and_times.insert(solution_and_times.end(), new_solution);
			}

		}

		outputFile.open("FGW_show_randD_effects_" + instance_name + ".csv");
		for (int j = 0; j < solution_and_times.size(); j++) {
			outputFile << solution_and_times[j].first << "," << solution_and_times[j].second << endl;
		}

	}
	outputFile.close();
	
}
#pragma endregion solve_hand_FGW_show_randD_effects


#pragma region
void solve_hand_FGW_randD() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 10; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;
	outputFile.open("Result_FGW1_hand.csv");
	outputFile << "Result_FGW1_hand.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 49; name++) {

		// instance_name
		if (name < 10) {
			instance = "handbd";
			instance_name = instance + to_string(0) + to_string(name);
		}
		else if (name < 15) {
			instance = "handbd";
			instance_name = instance + to_string(name);
		}
		else if (name < 24) {
			instance = "handbi";
			instance_name = instance + to_string(0) + to_string(name - 14);
		}
		else if (name < 29) {
			instance = "handbi";
			instance_name = instance + to_string(name - 14);
		}
		else if (name < 38) {
			instance = "handsd";
			instance_name = instance + to_string(0) + to_string(name - 28);
		}
		else if (name == 38) {
			instance = "handsd";
			instance_name = instance + to_string(name - 28);
		}
		else if (name < 48) {
			instance = "handsi";
			instance_name = instance + to_string(0) + to_string(name - 38);
		}
		else if (name == 48) {
			instance = "handsi";
			instance_name = instance + to_string(name - 38);
		}

		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			if (name == 1) {
				distribution_ratio = 2.4737;
			}
			else if (name == 2) {
				distribution_ratio = 3.1053;
			}
			else if (name == 3) {
				distribution_ratio = 15;
			}
			else if (name == 4) {
				distribution_ratio = 1.8021;
			}
			else if (name == 5) {
				distribution_ratio = 14;
			}
			else if (name == 6) {
				distribution_ratio = 1.85;
			}
			else if (name == 7) {
				distribution_ratio = 3.1053;
			}
			else if (name == 8) {
				distribution_ratio = 22.711;
			}
			else if (name == 9) {
				distribution_ratio = 1.4211;
			}
			else if (name == 10) {
				distribution_ratio = 1.4211;
			}
			else if (name == 11) {
				distribution_ratio = 1.0000;
			}
			else if (name == 12) {
				distribution_ratio = 9;
			}
			else if (name == 13) {
				distribution_ratio = 2.8947;
			}
			else if (name == 14) {
				distribution_ratio = 2.0526;
			}
			else if (name == 15) {
				distribution_ratio = 2.0526;
			}
			else if (name == 16) {
				distribution_ratio = 1.9187;
			}
			else if (name == 17) {
				distribution_ratio = 1.6316;
			}
			else if (name == 18) {
				distribution_ratio = 3.1053;
			}
			else if (name == 19) {
				distribution_ratio = 1.0000;
			}
			else if (name == 20) {
				distribution_ratio = 4.1579;
			}
			else if (name == 21) {
				distribution_ratio = 2.6842;
			}
			else if (name == 22) {
				distribution_ratio = 1.0204;
			}
			else if (name == 23) {
				distribution_ratio = 2.2632;
			}
			else if (name == 24) {
				distribution_ratio = 3.3158;
			}
			else if (name == 25) {
				distribution_ratio = 1.0000;
			}
			else if (name == 26) {
				distribution_ratio = 6.1913;
			}
			else if (name == 27) {
				distribution_ratio = 2.0526;
			}
			else if (name == 28) {
				distribution_ratio = 2.0526;
			}
			else if (name == 29) {
				distribution_ratio = 1.8421;
			}
			else if (name == 30) {
				distribution_ratio = 1.4211;
			}
			else if (name == 31) {
				distribution_ratio = 1.8421;
			}
			else if (name == 32) {
				distribution_ratio = 3.9474;
			}
			else if (name == 33) {
				distribution_ratio = 1.0000;
			}
			else if (name == 34) {
				distribution_ratio = 1.6316;
			}
			else if (name == 35) {
				distribution_ratio = 1.0000;
			}
			else if (name == 36) {
				distribution_ratio = 1.2105;
			}
			else if (name == 37) {
				distribution_ratio = 1.0000;
			}
			else if (name == 38) {
				distribution_ratio = 2;
			}
			else if (name == 39) {
				distribution_ratio = 1.6316;
			}
			else if (name == 40) {
				distribution_ratio = 2;
			}
			else if (name == 41) {
				distribution_ratio = 1.0000;
			}
			else if (name == 42) {
				distribution_ratio = 9;
			}
			else if (name == 43) {
				distribution_ratio = 1.0000;
			}
			else if (name == 44) {
				distribution_ratio = 2;
			}
			else if (name == 45) {
				distribution_ratio = 1.0000;
			}
			else if (name == 46) {
				distribution_ratio = 3.9474;
			}
			else if (name == 47) {
				distribution_ratio = 8;
			}
			else if (name == 48) {
				distribution_ratio = 1.2105;
			}


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
			//cout << "Growth Time= " << growth_time << "ms" << endl;
			// pruning Growth_graph
			GPrA_graph = GPrA(Growth_graph, GPrA_time);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
			//std::cout << "GAP: " << GAP << "\%" << endl;
			// total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / 10;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time/1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_hand_FGW_randD


#pragma region
void solve_hand_FGW_original_randD() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 1; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;
	outputFile.open("Result_FGW_original_hand.csv");
	outputFile << "Result_FGW1_hand.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 49; name++) {

		// instance_name
		if (name < 10) {
			instance = "handbd";
			instance_name = instance + to_string(0) + to_string(name);
		}
		else if (name < 15) {
			instance = "handbd";
			instance_name = instance + to_string(name);
		}
		else if (name < 24) {
			instance = "handbi";
			instance_name = instance + to_string(0) + to_string(name - 14);
		}
		else if (name < 29) {
			instance = "handbi";
			instance_name = instance + to_string(name - 14);
		}
		else if (name < 38) {
			instance = "handsd";
			instance_name = instance + to_string(0) + to_string(name - 28);
		}
		else if (name == 38) {
			instance = "handsd";
			instance_name = instance + to_string(name - 28);
		}
		else if (name < 48) {
			instance = "handsi";
			instance_name = instance + to_string(0) + to_string(name - 38);
		}
		else if (name == 48) {
			instance = "handsi";
			instance_name = instance + to_string(name - 38);
		}

		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			if (name == 1) {
				distribution_ratio = 2.4737;
			}
			else if (name == 2) {
				distribution_ratio = 3.1053;
			}
			else if (name == 3) {
				distribution_ratio = 15;
			}
			else if (name == 4) {
				distribution_ratio = 1.8021;
			}
			else if (name == 5) {
				distribution_ratio = 14;
			}
			else if (name == 6) {
				distribution_ratio = 1.85;
			}
			else if (name == 7) {
				distribution_ratio = 3.1053;
			}
			else if (name == 8) {
				distribution_ratio = 22.711;
			}
			else if (name == 9) {
				distribution_ratio = 1.4211;
			}
			else if (name == 10) {
				distribution_ratio = 1.4211;
			}
			else if (name == 11) {
				distribution_ratio = 1.0000;
			}
			else if (name == 12) {
				distribution_ratio = 9;
			}
			else if (name == 13) {
				distribution_ratio = 2.8947;
			}
			else if (name == 14) {
				distribution_ratio = 2.0526;
			}
			else if (name == 15) {
				distribution_ratio = 2.0526;
			}
			else if (name == 16) {
				distribution_ratio = 1.9187;
			}
			else if (name == 17) {
				distribution_ratio = 1.6316;
			}
			else if (name == 18) {
				distribution_ratio = 3.1053;
			}
			else if (name == 19) {
				distribution_ratio = 1.0000;
			}
			else if (name == 20) {
				distribution_ratio = 4.1579;
			}
			else if (name == 21) {
				distribution_ratio = 2.6842;
			}
			else if (name == 22) {
				distribution_ratio = 1.0204;
			}
			else if (name == 23) {
				distribution_ratio = 2.2632;
			}
			else if (name == 24) {
				distribution_ratio = 3.3158;
			}
			else if (name == 25) {
				distribution_ratio = 1.0000;
			}
			else if (name == 26) {
				distribution_ratio = 6.1913;
			}
			else if (name == 27) {
				distribution_ratio = 2.0526;
			}
			else if (name == 28) {
				distribution_ratio = 2.0526;
			}
			else if (name == 29) {
				distribution_ratio = 1.8421;
			}
			else if (name == 30) {
				distribution_ratio = 1.4211;
			}
			else if (name == 31) {
				distribution_ratio = 1.8421;
			}
			else if (name == 32) {
				distribution_ratio = 3.9474;
			}
			else if (name == 33) {
				distribution_ratio = 1.0000;
			}
			else if (name == 34) {
				distribution_ratio = 1.6316;
			}
			else if (name == 35) {
				distribution_ratio = 1.0000;
			}
			else if (name == 36) {
				distribution_ratio = 1.2105;
			}
			else if (name == 37) {
				distribution_ratio = 1.0000;
			}
			else if (name == 38) {
				distribution_ratio = 2;
			}
			else if (name == 39) {
				distribution_ratio = 1.6316;
			}
			else if (name == 40) {
				distribution_ratio = 2;
			}
			else if (name == 41) {
				distribution_ratio = 1.0000;
			}
			else if (name == 42) {
				distribution_ratio = 9;
			}
			else if (name == 43) {
				distribution_ratio = 1.0000;
			}
			else if (name == 44) {
				distribution_ratio = 2;
			}
			else if (name == 45) {
				distribution_ratio = 1.0000;
			}
			else if (name == 46) {
				distribution_ratio = 3.9474;
			}
			else if (name == 47) {
				distribution_ratio = 8;
			}
			else if (name == 48) {
				distribution_ratio = 1.2105;
			}


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = SPA_random_R(Growth_graph, GPrA_time);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
			//std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / times;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_hand_FGW_original_randD


#pragma region
void solve_hand_FGW_original_multiR_randD() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 1; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;
	outputFile.open("Result_FGW_original_hand.csv");
	outputFile << "Result_FGW1_hand.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 49; name++) {

		// instance_name
		if (name < 10) {
			instance = "handbd";
			instance_name = instance + to_string(0) + to_string(name);
		}
		else if (name < 15) {
			instance = "handbd";
			instance_name = instance + to_string(name);
		}
		else if (name < 24) {
			instance = "handbi";
			instance_name = instance + to_string(0) + to_string(name - 14);
		}
		else if (name < 29) {
			instance = "handbi";
			instance_name = instance + to_string(name - 14);
		}
		else if (name < 38) {
			instance = "handsd";
			instance_name = instance + to_string(0) + to_string(name - 28);
		}
		else if (name == 38) {
			instance = "handsd";
			instance_name = instance + to_string(name - 28);
		}
		else if (name < 48) {
			instance = "handsi";
			instance_name = instance + to_string(0) + to_string(name - 38);
		}
		else if (name == 48) {
			instance = "handsi";
			instance_name = instance + to_string(name - 38);
		}

		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			if (name == 1) {
				distribution_ratio = 2.4737;
			}
			else if (name == 2) {
				distribution_ratio = 3.1053;
			}
			else if (name == 3) {
				distribution_ratio = 15;
			}
			else if (name == 4) {
				distribution_ratio = 1.8021;
			}
			else if (name == 5) {
				distribution_ratio = 14;
			}
			else if (name == 6) {
				distribution_ratio = 1.85;
			}
			else if (name == 7) {
				distribution_ratio = 3.1053;
			}
			else if (name == 8) {
				distribution_ratio = 22.711;
			}
			else if (name == 9) {
				distribution_ratio = 1.4211;
			}
			else if (name == 10) {
				distribution_ratio = 1.4211;
			}
			else if (name == 11) {
				distribution_ratio = 1.0000;
			}
			else if (name == 12) {
				distribution_ratio = 9;
			}
			else if (name == 13) {
				distribution_ratio = 2.8947;
			}
			else if (name == 14) {
				distribution_ratio = 2.0526;
			}
			else if (name == 15) {
				distribution_ratio = 2.0526;
			}
			else if (name == 16) {
				distribution_ratio = 1.9187;
			}
			else if (name == 17) {
				distribution_ratio = 1.6316;
			}
			else if (name == 18) {
				distribution_ratio = 3.1053;
			}
			else if (name == 19) {
				distribution_ratio = 1.0000;
			}
			else if (name == 20) {
				distribution_ratio = 4.1579;
			}
			else if (name == 21) {
				distribution_ratio = 2.6842;
			}
			else if (name == 22) {
				distribution_ratio = 1.0204;
			}
			else if (name == 23) {
				distribution_ratio = 2.2632;
			}
			else if (name == 24) {
				distribution_ratio = 3.3158;
			}
			else if (name == 25) {
				distribution_ratio = 1.0000;
			}
			else if (name == 26) {
				distribution_ratio = 6.1913;
			}
			else if (name == 27) {
				distribution_ratio = 2.0526;
			}
			else if (name == 28) {
				distribution_ratio = 2.0526;
			}
			else if (name == 29) {
				distribution_ratio = 1.8421;
			}
			else if (name == 30) {
				distribution_ratio = 1.4211;
			}
			else if (name == 31) {
				distribution_ratio = 1.8421;
			}
			else if (name == 32) {
				distribution_ratio = 3.9474;
			}
			else if (name == 33) {
				distribution_ratio = 1.0000;
			}
			else if (name == 34) {
				distribution_ratio = 1.6316;
			}
			else if (name == 35) {
				distribution_ratio = 1.0000;
			}
			else if (name == 36) {
				distribution_ratio = 1.2105;
			}
			else if (name == 37) {
				distribution_ratio = 1.0000;
			}
			else if (name == 38) {
				distribution_ratio = 2;
			}
			else if (name == 39) {
				distribution_ratio = 1.6316;
			}
			else if (name == 40) {
				distribution_ratio = 2;
			}
			else if (name == 41) {
				distribution_ratio = 1.0000;
			}
			else if (name == 42) {
				distribution_ratio = 9;
			}
			else if (name == 43) {
				distribution_ratio = 1.0000;
			}
			else if (name == 44) {
				distribution_ratio = 2;
			}
			else if (name == 45) {
				distribution_ratio = 1.0000;
			}
			else if (name == 46) {
				distribution_ratio = 3.9474;
			}
			else if (name == 47) {
				distribution_ratio = 8;
			}
			else if (name == 48) {
				distribution_ratio = 1.2105;
			}


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = random__multiR_SPA(Growth_graph, GPrA_time, 100);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
														 //std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / times;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_hand_FGW_original_multiR_randD


#pragma region
void solve_hand_FGW_randD_newRatio() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 10; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;
	outputFile.open("Result_FGW1_hand_newRatio.csv");
	outputFile << "Result_FGW1_hand_newRatio.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 49; name++) {

		// instance_name
		if (name < 10) {
			instance = "handbd";
			instance_name = instance + to_string(0) + to_string(name);
		}
		else if (name < 15) {
			instance = "handbd";
			instance_name = instance + to_string(name);
		}
		else if (name < 24) {
			instance = "handbi";
			instance_name = instance + to_string(0) + to_string(name - 14);
		}
		else if (name < 29) {
			instance = "handbi";
			instance_name = instance + to_string(name - 14);
		}
		else if (name < 38) {
			instance = "handsd";
			instance_name = instance + to_string(0) + to_string(name - 28);
		}
		else if (name == 38) {
			instance = "handsd";
			instance_name = instance + to_string(name - 28);
		}
		else if (name < 48) {
			instance = "handsi";
			instance_name = instance + to_string(0) + to_string(name - 38);
		}
		else if (name == 48) {
			instance = "handsi";
			instance_name = instance + to_string(name - 38);
		}

		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			if (name == 1) {
				distribution_ratio = 8.575;
			}
			else if (name == 2) {
				distribution_ratio = 4.752;
			}
			else if (name == 3) {
				distribution_ratio = 3.474;
			}
			else if (name == 4) {
				distribution_ratio = 1.8021;
			}
			else if (name == 5) {
				distribution_ratio = 14;
			}
			else if (name == 6) {
				distribution_ratio = 1.85;
			}
			else if (name == 7) {
				distribution_ratio = 3.1053;
			}
			else if (name == 8) {
				distribution_ratio = 32.097;
			}
			else if (name == 9) {
				distribution_ratio = 31.502;
			}
			else if (name == 10) {
				distribution_ratio = 1.4211;
			}
			else if (name == 11) {
				distribution_ratio = 1.0000;
			}
			else if (name == 12) {
				distribution_ratio = 23.922;
			}
			else if (name == 13) {
				distribution_ratio = 2.8947;
			}
			else if (name == 14) {
				distribution_ratio = 2.0526;
			}
			else if (name == 15) {
				distribution_ratio = 25.46;
			}
			else if (name == 16) {
				distribution_ratio = 1.9187;
			}
			else if (name == 17) {
				distribution_ratio = 1.6316;
			}
			else if (name == 18) {
				distribution_ratio = 3.1053;
			}
			else if (name == 19) {
				distribution_ratio = 1.0000;
			}
			else if (name == 20) {
				distribution_ratio = 4.1579;
			}
			else if (name == 21) {
				distribution_ratio = 2.6842;
			}
			else if (name == 22) {
				distribution_ratio = 1.0204;
			}
			else if (name == 23) {
				distribution_ratio = 2.2632;
			}
			else if (name == 24) {
				distribution_ratio = 7.746;
			}
			else if (name == 25) {
				distribution_ratio = 1.0000;
			}
			else if (name == 26) {
				distribution_ratio = 6.1913;
			}
			else if (name == 27) {
				distribution_ratio = 2.0526;
			}
			else if (name == 28) {
				distribution_ratio = 2.0526;
			}
			else if (name == 29) {
				distribution_ratio = 25.538;
			}
			else if (name == 30) {
				distribution_ratio = 1.4211;
			}
			else if (name == 31) {
				distribution_ratio = 1.8421;
			}
			else if (name == 32) {
				distribution_ratio = 32.441;
			}
			else if (name == 33) {
				distribution_ratio = 2.613;
			}
			else if (name == 34) {
				distribution_ratio = 1.511;
			}
			else if (name == 35) {
				distribution_ratio = 1.0000;
			}
			else if (name == 36) {
				distribution_ratio = 1.085;
			}
			else if (name == 37) {
				distribution_ratio = 1.0000;
			}
			else if (name == 38) {
				distribution_ratio = 2;
			}
			else if (name == 39) {
				distribution_ratio = 27.321;
			}
			else if (name == 40) {
				distribution_ratio = 2;
			}
			else if (name == 41) {
				distribution_ratio = 1.0000;
			}
			else if (name == 42) {
				distribution_ratio = 10.12;
			}
			else if (name == 43) {
				distribution_ratio = 1.0000;
			}
			else if (name == 44) {
				distribution_ratio = 7.824;
			}
			else if (name == 45) {
				distribution_ratio = 1.0000;
			}
			else if (name == 46) {
				distribution_ratio = 33.616;
			}
			else if (name == 47) {
				distribution_ratio = 8;
			}
			else if (name == 48) {
				distribution_ratio = 1.2105;
			}


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = GPrA(Growth_graph, GPrA_time);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
														 //std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / 10;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_hand_FGW_randD_newRatio


#pragma region
void solve_hand_FGW_original_randD_newRatio() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 1; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;
	outputFile.open("Result_FGW_original_hand.csv");
	outputFile << "Result_FGW1_hand.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 49; name++) {

		// instance_name
		if (name < 10) {
			instance = "handbd";
			instance_name = instance + to_string(0) + to_string(name);
		}
		else if (name < 15) {
			instance = "handbd";
			instance_name = instance + to_string(name);
		}
		else if (name < 24) {
			instance = "handbi";
			instance_name = instance + to_string(0) + to_string(name - 14);
		}
		else if (name < 29) {
			instance = "handbi";
			instance_name = instance + to_string(name - 14);
		}
		else if (name < 38) {
			instance = "handsd";
			instance_name = instance + to_string(0) + to_string(name - 28);
		}
		else if (name == 38) {
			instance = "handsd";
			instance_name = instance + to_string(name - 28);
		}
		else if (name < 48) {
			instance = "handsi";
			instance_name = instance + to_string(0) + to_string(name - 38);
		}
		else if (name == 48) {
			instance = "handsi";
			instance_name = instance + to_string(name - 38);
		}

		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			if (name == 1) {
				distribution_ratio = 8.575;
			}
			else if (name == 2) {
				distribution_ratio = 4.752;
			}
			else if (name == 3) {
				distribution_ratio = 3.474;
			}
			else if (name == 4) {
				distribution_ratio = 1.8021;
			}
			else if (name == 5) {
				distribution_ratio = 14;
			}
			else if (name == 6) {
				distribution_ratio = 1.85;
			}
			else if (name == 7) {
				distribution_ratio = 3.1053;
			}
			else if (name == 8) {
				distribution_ratio = 32.097;
			}
			else if (name == 9) {
				distribution_ratio = 31.502;
			}
			else if (name == 10) {
				distribution_ratio = 1.4211;
			}
			else if (name == 11) {
				distribution_ratio = 1.0000;
			}
			else if (name == 12) {
				distribution_ratio = 23.922;
			}
			else if (name == 13) {
				distribution_ratio = 2.8947;
			}
			else if (name == 14) {
				distribution_ratio = 2.0526;
			}
			else if (name == 15) {
				distribution_ratio = 25.46;
			}
			else if (name == 16) {
				distribution_ratio = 1.9187;
			}
			else if (name == 17) {
				distribution_ratio = 1.6316;
			}
			else if (name == 18) {
				distribution_ratio = 3.1053;
			}
			else if (name == 19) {
				distribution_ratio = 1.0000;
			}
			else if (name == 20) {
				distribution_ratio = 4.1579;
			}
			else if (name == 21) {
				distribution_ratio = 2.6842;
			}
			else if (name == 22) {
				distribution_ratio = 1.0204;
			}
			else if (name == 23) {
				distribution_ratio = 2.2632;
			}
			else if (name == 24) {
				distribution_ratio = 7.746;
			}
			else if (name == 25) {
				distribution_ratio = 1.0000;
			}
			else if (name == 26) {
				distribution_ratio = 6.1913;
			}
			else if (name == 27) {
				distribution_ratio = 2.0526;
			}
			else if (name == 28) {
				distribution_ratio = 2.0526;
			}
			else if (name == 29) {
				distribution_ratio = 25.538;
			}
			else if (name == 30) {
				distribution_ratio = 1.4211;
			}
			else if (name == 31) {
				distribution_ratio = 1.8421;
			}
			else if (name == 32) {
				distribution_ratio = 32.441;
			}
			else if (name == 33) {
				distribution_ratio = 2.613;
			}
			else if (name == 34) {
				distribution_ratio = 1.511;
			}
			else if (name == 35) {
				distribution_ratio = 1.0000;
			}
			else if (name == 36) {
				distribution_ratio = 1.085;
			}
			else if (name == 37) {
				distribution_ratio = 1.0000;
			}
			else if (name == 38) {
				distribution_ratio = 2;
			}
			else if (name == 39) {
				distribution_ratio = 27.321;
			}
			else if (name == 40) {
				distribution_ratio = 2;
			}
			else if (name == 41) {
				distribution_ratio = 1.0000;
			}
			else if (name == 42) {
				distribution_ratio = 10.12;
			}
			else if (name == 43) {
				distribution_ratio = 1.0000;
			}
			else if (name == 44) {
				distribution_ratio = 7.824;
			}
			else if (name == 45) {
				distribution_ratio = 1.0000;
			}
			else if (name == 46) {
				distribution_ratio = 33.616;
			}
			else if (name == 47) {
				distribution_ratio = 8;
			}
			else if (name == 48) {
				distribution_ratio = 1.2105;
			}


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = SPA_random_R(Growth_graph, GPrA_time);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
														 //std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / times;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_hand_FGW_original_randD_newRatio


#pragma region
void solve_hand_FGW_original_multiR_randD_newRatio() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 1; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;
	outputFile.open("Result_FGW_original_hand.csv");
	outputFile << "Result_FGW1_hand.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 49; name++) {

		// instance_name
		if (name < 10) {
			instance = "handbd";
			instance_name = instance + to_string(0) + to_string(name);
		}
		else if (name < 15) {
			instance = "handbd";
			instance_name = instance + to_string(name);
		}
		else if (name < 24) {
			instance = "handbi";
			instance_name = instance + to_string(0) + to_string(name - 14);
		}
		else if (name < 29) {
			instance = "handbi";
			instance_name = instance + to_string(name - 14);
		}
		else if (name < 38) {
			instance = "handsd";
			instance_name = instance + to_string(0) + to_string(name - 28);
		}
		else if (name == 38) {
			instance = "handsd";
			instance_name = instance + to_string(name - 28);
		}
		else if (name < 48) {
			instance = "handsi";
			instance_name = instance + to_string(0) + to_string(name - 38);
		}
		else if (name == 48) {
			instance = "handsi";
			instance_name = instance + to_string(name - 38);
		}

		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			if (name == 1) {
				distribution_ratio = 8.575;
			}
			else if (name == 2) {
				distribution_ratio = 4.752;
			}
			else if (name == 3) {
				distribution_ratio = 3.474;
			}
			else if (name == 4) {
				distribution_ratio = 1.8021;
			}
			else if (name == 5) {
				distribution_ratio = 14;
			}
			else if (name == 6) {
				distribution_ratio = 1.85;
			}
			else if (name == 7) {
				distribution_ratio = 3.1053;
			}
			else if (name == 8) {
				distribution_ratio = 32.097;
			}
			else if (name == 9) {
				distribution_ratio = 31.502;
			}
			else if (name == 10) {
				distribution_ratio = 1.4211;
			}
			else if (name == 11) {
				distribution_ratio = 1.0000;
			}
			else if (name == 12) {
				distribution_ratio = 23.922;
			}
			else if (name == 13) {
				distribution_ratio = 2.8947;
			}
			else if (name == 14) {
				distribution_ratio = 2.0526;
			}
			else if (name == 15) {
				distribution_ratio = 25.46;
			}
			else if (name == 16) {
				distribution_ratio = 1.9187;
			}
			else if (name == 17) {
				distribution_ratio = 1.6316;
			}
			else if (name == 18) {
				distribution_ratio = 3.1053;
			}
			else if (name == 19) {
				distribution_ratio = 1.0000;
			}
			else if (name == 20) {
				distribution_ratio = 4.1579;
			}
			else if (name == 21) {
				distribution_ratio = 2.6842;
			}
			else if (name == 22) {
				distribution_ratio = 1.0204;
			}
			else if (name == 23) {
				distribution_ratio = 2.2632;
			}
			else if (name == 24) {
				distribution_ratio = 7.746;
			}
			else if (name == 25) {
				distribution_ratio = 1.0000;
			}
			else if (name == 26) {
				distribution_ratio = 6.1913;
			}
			else if (name == 27) {
				distribution_ratio = 2.0526;
			}
			else if (name == 28) {
				distribution_ratio = 2.0526;
			}
			else if (name == 29) {
				distribution_ratio = 25.538;
			}
			else if (name == 30) {
				distribution_ratio = 1.4211;
			}
			else if (name == 31) {
				distribution_ratio = 1.8421;
			}
			else if (name == 32) {
				distribution_ratio = 32.441;
			}
			else if (name == 33) {
				distribution_ratio = 2.613;
			}
			else if (name == 34) {
				distribution_ratio = 1.511;
			}
			else if (name == 35) {
				distribution_ratio = 1.0000;
			}
			else if (name == 36) {
				distribution_ratio = 1.085;
			}
			else if (name == 37) {
				distribution_ratio = 1.0000;
			}
			else if (name == 38) {
				distribution_ratio = 2;
			}
			else if (name == 39) {
				distribution_ratio = 27.321;
			}
			else if (name == 40) {
				distribution_ratio = 2;
			}
			else if (name == 41) {
				distribution_ratio = 1.0000;
			}
			else if (name == 42) {
				distribution_ratio = 10.12;
			}
			else if (name == 43) {
				distribution_ratio = 1.0000;
			}
			else if (name == 44) {
				distribution_ratio = 7.824;
			}
			else if (name == 45) {
				distribution_ratio = 1.0000;
			}
			else if (name == 46) {
				distribution_ratio = 33.616;
			}
			else if (name == 47) {
				distribution_ratio = 8;
			}
			else if (name == 48) {
				distribution_ratio = 1.2105;
			}


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = random__multiR_SPA(Growth_graph, GPrA_time, 100);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
														 //std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / times;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_hand_FGW_original_multiR_randD_newRatio





#pragma region
void solve_PPI_FGW_randD() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 10; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;

	// write file
	outputFile.open("Result_FGW1_PPI.csv");
	outputFile << "Result_FGW1_PPI.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 11; name++) {

		// instance_name
		instance = "PPI";
		instance_name = instance + "_" + to_string(name);

		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			distribution_ratio = 2;


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = GPrA(Growth_graph, GPrA_time);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
														 //std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			if (min_time > Total_time) {
				min_time = Total_time;
			}
			if (max_time < Total_time) {
				max_time = Total_time;
			}
			avg_time = avg_time + Total_time;
		}
		avg_time = (avg_time - min_time - max_time) / 8;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_PPI_FGW_randD


#pragma region
void solve_DSN_FGW_randD() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 10; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;

						   // write file
	outputFile.open("Result_FGW1_DSN.csv");
	outputFile << "Result_FGW1_DSN.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 21; name++) {

		if (name != 6 && name != 16) {
			// instance_name
			instance = "D";
			if (name < 10) {
				instance_name = instance + "_0" + to_string(name) + "_a";
			}
			else if (name == 10) {
				instance_name = instance + "_" + to_string(name) + "_a";
			}
			else if (name < 20) {
				instance_name = instance + "_0" + to_string(name - 10) + "_b";
			}
			else if (name == 20) {
				instance_name = instance + "_" + to_string(name - 10) + "_b";
			}

			input_graph = read_data_with_terminals(instance_name + ".stp");
			min_time = 1e8;
			double max_time = 1e-8;
			double avg_time = 0;

			for (int i = 0; i < times; i++) {

				// get D
				distribution_ratio = 2;


				// FGW find subgraph
				Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																				  //cout << "Growth Time= " << growth_time << "ms" << endl;
																				  // pruning Growth_graph
				GPrA_graph = GPrA(Growth_graph, GPrA_time);
				Solu_net_cost = net_cost(GPrA_graph);
				cout << "cost= " << Solu_net_cost << endl;
				//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
				//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
				// GW_LB
				GW_lb = GW_LB(GPrA_graph);
				//std::cout << "GW_LB: " << GW_lb << endl;
				GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
															 //std::cout << "GAP: " << GAP << "\%" << endl;
															 // total time
				Total_time = growth_time + GPrA_time;
				cout << "Total_time= " << Total_time << "ms" << endl;

				if (min_time > Total_time) {
					min_time = Total_time;
				}
				if (max_time < Total_time) {
					max_time = Total_time;
				}
				avg_time = avg_time + Total_time;
			}
			avg_time = (avg_time - min_time - max_time) / 8;
			//save_data
			cout << endl;
			cout << "Solu_net_cost= " << Solu_net_cost << endl;
			cout << "avg_time= " << avg_time << "ms" << endl;
			save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

			outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
				<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
		}
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_DSN_FGW_randD


#pragma region
void solve_M_FGW_randD() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 1; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;

						   // write file
	instance = "M";
	outputFile.open("Result_FGW_M.csv");
	outputFile << "Result_FGW_M.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 41; name++) {

		string instance_name; // instance_name
		if (name < 21) {
			instance_name = instance + to_string(name) + "A";
		}
		else {
			instance_name = instance + to_string(name - 20) + "B";
		}
		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			distribution_ratio = 2;


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = GPrA(Growth_graph, GPrA_time);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
														 //std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / times;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_M_FGW_randD


#pragma region
void solve_M_FGW_original_randD() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 1; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;

	// write file
	instance = "M";
	outputFile.open("Result_FGW_original_M.csv");
	outputFile << "Result_FGW_original_M.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 41; name++) {

		string instance_name; // instance_name
		if (name < 21) {
			instance_name = instance + to_string(name) + "A";
		}
		else {
			instance_name = instance + to_string(name - 20) + "B";
		}
		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			distribution_ratio = 2;


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = SPA_random_R(Growth_graph, GPrA_time);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
														 //std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / times;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_M_FGW_original_randD


#pragma region
void solve_M_FGW_original_multiR_randD() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 1; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	double distribution_ratio;

	// write file
	instance = "M";
	outputFile.open("Result_FGW_original_M.csv");
	outputFile << "Result_FGW_original_M.csv" << endl;
	outputFile << "Instance,|V|,|E|,Time,Cost,GW_LB,Gap" << endl;

	for (int name = 1; name < 41; name++) {

		string instance_name; // instance_name
		if (name < 21) {
			instance_name = instance + to_string(name) + "A";
		}
		else {
			instance_name = instance + to_string(name - 20) + "B";
		}
		input_graph = read_data_with_terminals(instance_name + ".stp");
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;

		for (int i = 0; i < times; i++) {

			// get D
			distribution_ratio = 2;


			// FGW find subgraph
			Growth_graph = FGW(input_graph, growth_time, distribution_ratio); // FGW find subgraph
																			  //cout << "Growth Time= " << growth_time << "ms" << endl;
																			  // pruning Growth_graph
			GPrA_graph = random__multiR_SPA(Growth_graph, GPrA_time, 100);
			Solu_net_cost = net_cost(GPrA_graph);
			cout << "cost= " << Solu_net_cost << endl;
			//cout << "GPrA Time= " << GPrA_time << "ms" << endl;
			//std::cout << "GPrA_graph net_cost: " << net_cost(GPrA_graph) << endl;
			// GW_LB
			GW_lb = GW_LB(GPrA_graph);
			//std::cout << "GW_LB: " << GW_lb << endl;
			GAP = (Solu_net_cost - GW_lb) / GW_lb * 100; // %
														 //std::cout << "GAP: " << GAP << "\%" << endl;
														 // total time
			Total_time = growth_time + GPrA_time;
			cout << "Total_time= " << Total_time << "ms" << endl;

			avg_time = avg_time + Total_time;
		}
		avg_time = avg_time / times;
		//save_data
		cout << endl;
		cout << "Solu_net_cost= " << Solu_net_cost << endl;
		cout << "avg_time= " << avg_time << "ms" << endl;
		save_data_FGW(instance_name, GPrA_graph, Solu_net_cost, avg_time, distribution_ratio);

		outputFile << instance_name << "," << num_vertices(input_graph) << "," << num_edges(input_graph) << ","
			<< avg_time / 1000 << "s" << "," << Solu_net_cost << "," << GW_lb << "," << GAP << "\%" << endl;
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion solve_M_FGW_original_multiR_randD


#pragma region
void check_event_num() {

	string instance_name; // instance_name
	string instance;
	ofstream outputFile;
	outputFile.precision(4);
	outputFile.setf(ios::fixed);
	outputFile.setf(ios::showpoint);
	double growth_time;
	double GPrA_time;
	double Total_time;
	double GW_lb;
	double GAP;
	double min_time;
	double times = 100; // run each instance for x times
	graph input_graph;
	graph GPrA_graph;
	graph Growth_graph;
	double Solu_net_cost;

	// write file
	outputFile.open("Result_Event_num.csv");
	outputFile << "Result_Event_num.csv" << endl;

	for (int name = 1; name < 2; name++) {

		// instance_name
		instance = "PPI";
		instance_name = instance + "_" + to_string(name);

		input_graph = read_data_with_terminals(instance_name + ".stp");
		outputFile << "Instance: "+ instance_name << endl;
		outputFile << "D,edge_event_num,cluster_event_num,total_event_num" << endl;
		min_time = 1e8;
		double max_time = 1e-8;
		double avg_time = 0;
		times = 300;
		for (int i = 0; i < times; i++) {

			// get D
			double x = i;
			double distribution_ratio = 1 + x/5;
			cout << i << " " << distribution_ratio << endl;
			// FGW find subgraph
			int edge_event_num;
			int cluster_event_num;
			int total_event_num;
			Growth_graph = FGW_eventsNum(input_graph, growth_time, distribution_ratio, edge_event_num, cluster_event_num, total_event_num); // FGW find subgraph
			outputFile << distribution_ratio << "," << edge_event_num << "," << cluster_event_num << "," << total_event_num << endl;
		}
	}

	outputFile.close();
	std::cout << "END" << endl;
}
#pragma endregion check_event_num


#pragma region
int main() {

	srand((unsigned)time(NULL));

	std::cout << std::setprecision(4) << std::fixed;

	solve_hand_FGW_original_multiR_randD_newRatio();

	std::cout << "END" << endl;
	getchar();

}
#pragma endregion main method
