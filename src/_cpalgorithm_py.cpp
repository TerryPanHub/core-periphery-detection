#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <algorithm>
#include <iostream>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

/* -----------------------------------------------------
Write the algorithms to be included in the prackage here
----------------------------------------------------- */
#include <bealgorithm.h>
#include <minres.h>
#include <km_config.h>
#include <km_modmat.h>

using namespace std;
namespace py = pybind11;

void readEdgeTable(py::array_t<int> edges_array_t, py::array_t<double> w_array_t, Graph& G)
{

    vector<int> edgeList;
    vector<double> wList;
    int N = 0;	
    auto edges = edges_array_t.data();
    auto r = edges_array_t.request();
    int M = r.shape[0];
    auto ws = w_array_t.data();
	
    for(int i =0; i< M; i++){
        int sid = edges[2*i];
        int did = edges[2*i + 1];
	double w = ws[i];
        if (sid == did)
            continue;
        if (N < sid)
            N = sid;
        if (N < did)
            N = did;
        edgeList.push_back(sid);
        edgeList.push_back(did);
        wList.push_back(w);
    }
    N = N + 1;
  
    Graph tmp(N);
    G = tmp; 

    int wid = 0; 
    int edgeListsize = edgeList.size();
    for (int i = 0; i < edgeListsize; i += 2) {
        int sid = edgeList[i];
        int did = edgeList[i + 1];
	double w = wList[wid];
	G.addEdge(sid, did, w);
	wid++;
    }
}

void readCPResult(py::array_t<int> c_array_t, py::array_t<bool> x_array_t, vector<int>& c, vector<bool>& x)
{

    auto _c = c_array_t.data();
    auto _x = x_array_t.data();
    
    auto r = c_array_t.request();
    int N = r.shape[0];

    vector<int> cList(N, 0.0);
    vector<bool> xList(N, false);
	
    for(int i =0; i< N; i++){
        cList[i] = _c[i];
        xList[i] = _x[i];
    }
    c = cList;
    x = xList;
}

void packResults(vector<int>&c, vector<bool>& x, double& Q, vector<double>&q, py::list& results)
{
	int N = c.size();
	py::array_t<double> cids_array_t(N);
	auto cids = cids_array_t.mutable_data();
	
	py::array_t<double> xs_array_t(N);
	auto xs = xs_array_t.mutable_data();
	
	for(int i = 0; i < N; i++){
		cids[i] = c[i];
		xs[i] = x[i];
	}
	
	int K = q.size();
	py::array_t<double> qs_array_t(K);
	auto qs = qs_array_t.mutable_data();
	
	for(int i = 0; i < K; i++){
		qs[i] = q[i];
	}
	
	py::array_t<double> Qs_array_t(1);
	auto Qs = Qs_array_t.mutable_data();
	
	Qs[0] = Q;	

	//py::list results(3);
	results[0] = cids_array_t;
	results[1] = xs_array_t;
	results[2] = Qs_array_t;
	results[3] = qs_array_t;
}

void pack_Q(double _Q, vector<double>& _q, py::list& results)
{
	int K = _q.size();
	py::array_t<double> q_array_t(K);
	auto q = q_array_t.mutable_data();
	
	py::array_t<double> Q_array_t(1);
	auto Q = Q_array_t.mutable_data();
	
	Q[0] = _Q;
	for(int i = 0; i < K; i++){
		q[i] = _q[i];
	}
	results[0] = Q_array_t;
	results[1] = q_array_t;
}

/* BE algorithm*/
py::list detect_be(py::array_t<int> edges, py::array_t<double> ws, int num_of_runs){
	
       	Graph G(0); 
	readEdgeTable(edges, ws, G);

	BEAlgorithm be = BEAlgorithm(num_of_runs);

	be.detect(G);
	
	double Q = 0;
	vector<double>q;
	be._calc_Q(G, Q, q);
	
	vector<int>  c = be.get_c();
	vector<bool> x = be.get_x();

	py::list results(4);
	packResults(c, x, Q, q, results);	

	return results;
}

/* MINRES algorithm*/
py::list detect_minres(py::array_t<int> edges, py::array_t<double> ws, int num_of_runs){
	
       	Graph G(0); 
	readEdgeTable(edges, ws, G);

	MINRES minres = MINRES();

	minres.detect(G);

	double Q = 0;
	vector<double>q;
	minres._calc_Q(G, Q, q);
	
	vector<int>  c = minres.get_c();
	vector<bool> x = minres.get_x();

	py::list results(4);
	packResults(c, x, Q, q, results);	

	return results;
}
/* KM algorithm based on the configuration model */
py::list detect_config(py::array_t<int> edges, py::array_t<double> ws, int num_of_runs){
	
       	Graph G(0); 
	readEdgeTable(edges, ws, G);

        //mt19937_64 mtrnd = init_random_number_generator();
	KM_config km = KM_config(num_of_runs);
	km.detect(G);
	
	double Q = 0;
	vector<double>q;
	km._calc_Q(G, Q, q);
	
	vector<int>c = km.get_c();
	vector<bool>x = km.get_x();

	py::list results(4);

	packResults(c, x, Q, q, results);
	return results;
}

/* KM algorithm based on an arbitary null models*/
py::list detect_modmat(py::array_t<int> edges, py::array_t<double> ws, int num_of_runs){
	
       	Graph G(0); 
	readEdgeTable(edges, ws, G);
	KM_modmat km = KM_modmat(num_of_runs);

	km.detect(G);
	
	double Q = 0;
	vector<double>q;
	km._calc_Q(G, Q, q);
	
	vector<int>  c = km.get_c();
	vector<bool> x = km.get_x();

	py::list results(4);
	packResults(c, x, Q, q, results);	

	return results;
}

py::list calc_Q_config(py::array_t<int> edges, py::array_t<double> ws, py::array_t<int> _c, py::array_t<bool> _x){

	vector<int> c;	
	vector<bool> x;	
       	Graph G(0); 
	readEdgeTable(edges, ws, G);
	readCPResult(_c, _x, c, x);
	
	KM_config km = KM_config();

	double Q = -1;
	vector<double>q;
	km.calc_Q(G, c, x, Q, q);
	
	py::list results(2);
	pack_Q(Q, q, results);	
	return results;
}

py::list calc_Q_minres(py::array_t<int> edges, py::array_t<double> ws, py::array_t<int> _c, py::array_t<bool> _x){

	vector<int> c;	
	vector<bool> x;	
       	Graph G(0); 
	readEdgeTable(edges, ws, G);
	readCPResult(_c, _x, c, x);
	
	MINRES km = MINRES();

	double Q = -1;
	vector<double>q;
	km.calc_Q(G, c, x, Q, q);
	
	py::list results(2);
	pack_Q(Q, q, results);	
	return results;
}

py::list calc_Q_be(py::array_t<int> edges, py::array_t<double> ws, py::array_t<int> _c, py::array_t<bool> _x){

	vector<int> c;	
	vector<bool> x;	
       	Graph G(0); 
	readEdgeTable(edges, ws, G);
	readCPResult(_c, _x, c, x);
	
	BEAlgorithm km = BEAlgorithm();

	double Q = -1;
	vector<double>q;
	km.calc_Q(G, c, x, Q, q);
	
	py::list results(2);
	pack_Q(Q, q, results);	
	return results;
}

py::list calc_Q_modmat(py::array_t<int> edges, py::array_t<double> ws, py::array_t<int> _c, py::array_t<bool> _x){

	vector<int> c;	
	vector<bool> x;	
       	Graph G(0); 
	readEdgeTable(edges, ws, G);
	readCPResult(_c, _x, c, x);
	
	KM_modmat km = KM_modmat();

	double Q = -1;
	vector<double>q;
	km.calc_Q(G, c, x, Q, q);
	
	py::list results(2);
	pack_Q(Q, q, results);	
	return results;
}

PYBIND11_MODULE(_cpalgorithm, m){
	m.doc() = "Core-periphery detection in networks";

	// CP detection algorithm 
	m.def("detect_be", &detect_be, "Borgatti-Everett algorithm",
		py::arg("edges"),
		py::arg("ws"),
		py::arg("num_of_runs") = 10
	);

	m.def("detect_minres", &detect_minres, "MINRES algorithm",
		py::arg("edges"),
		py::arg("ws"),
		py::arg("num_of_runs") = 10
	);

	m.def("detect_config", &detect_config, "Use the configuration model as null models",
		py::arg("edges"),
		py::arg("ws"),
		py::arg("num_of_runs") = 10
	);

	m.def("detect_modmat", &detect_modmat, "Use the user-provided modularity matrix",
		py::arg("edges"),
		py::arg("ws"),
		py::arg("num_of_runs") = 10
	);

	// Quality functions
	m.def("calc_Q_be", &calc_Q_be, "Borgatti-Everett algorithm",
		py::arg("edges"),
		py::arg("ws"),
		py::arg("c"),
		py::arg("x")
	);

	m.def("calc_Q_minres", &calc_Q_minres, "MINRES algorithm",
		py::arg("edges"),
		py::arg("ws"),
		py::arg("c"),
		py::arg("x")
	);

	m.def("calc_Q_config", &calc_Q_config, "Use the configuration model as null models",
		py::arg("edges"),
		py::arg("ws"),
		py::arg("c"),
		py::arg("x")
	);

	m.def("calc_Q_modmat", &calc_Q_modmat, "Use the user-provided modularity matrix",
		py::arg("edges"),
		py::arg("ws"),
		py::arg("c"),
		py::arg("x")
	);
}