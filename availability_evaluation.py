"""
    Consist of all availability evaluation methods
"""
from itertools import combinations
import os
import pickle as pkl
import networkx as nx
import networkx as nx
from itertools import combinations
import build.rbd_bindings

from cutsets import optimized_minimalcuts

# Load data from a pickle file
def read_graph(directory, top):
    with open(os.path.join(directory, 'Pickle_' + top + '.pickle'), 'rb') as handle:
        f = pkl.load(handle)
    G = f[0]
    pos = f[1]
    lable = f[2]

    return G, pos, lable

# Relabel the nodes of G and A_dic
def relabel_graph_A_dict(G, A_dic):
    # Get the nodes of G
    nodes = list(G.nodes())
    # Create a mapping of the nodes to new labels
    relabel_mapping = {nodes[i]: i + 1 for i in range(len(nodes))}
    # Relabel the nodes of G
    G_relabel = nx.relabel_nodes(G, relabel_mapping)
    # Relabel the nodes in A_dict
    A_dic = {relabel_mapping[node]: value for node, value in A_dic.items()}
    return G_relabel, A_dic, relabel_mapping

def calculate_availability_cpp(G, source, target, A_dic):
    
    # Relabel the nodes of G and A_dic
    G, A_dic, relabel_mapping = relabel_graph_A_dict(G, A_dic)
    
    # Find the new source and target
    source = relabel_mapping[source]
    target = relabel_mapping[target]
    
    # Calculate the availability
    result = build.rbd_bindings.evaluateAvailability(optimized_minimalcuts(G, source, target), A_dic, source, target)
    return (source, target, result)
    

def calculate_availability_multiprocessing_cpp(G, A_dic):
    # Relabel the nodes of G and A_dic
    G, A_dic, _ = relabel_graph_A_dict(G, A_dic)
    node_pairs = list(combinations(G.nodes(), 2))
    mincutsets = [optimized_minimalcuts(G, pair[0], pair[1]) for pair in node_pairs]
    results = build.rbd_bindings.evaluateAvailabilityTopologyMultiProcessing(mincutsets, A_dic, node_pairs)
    
    return results

def calculate_availability_multithreading_cpp(G, A_dic):
    # Relabel the nodes of G and A_dic
    G, A_dic, _ = relabel_graph_A_dict(G, A_dic)
    node_pairs = list(combinations(G.nodes(), 2))
    mincutsets = [optimized_minimalcuts(G, pair[0], pair[1]) for pair in node_pairs]
    results = build.rbd_bindings.evaluateAvailabilityTopologyMultiThreading(mincutsets, A_dic, node_pairs)
    
    return results