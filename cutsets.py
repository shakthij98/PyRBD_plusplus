import networkx as nx
from itertools import combinations, islice
import numpy as np
import pandas as pd
import time
import logging


def successpaths(H, source, target, weight='weight'):
    return list(nx.shortest_simple_paths(H, source, target, weight=weight))

# Function for Finding Minimal cuts improved
def optimized_minimalcuts(H, src_, dst_, order=6):
    # save the minimal cut sets
    minimal = []
 
    if nx.shortest_path_length(H, source=src_, target=dst_) > 1:
        # get all paths from source to destination
        paths = successpaths(H, src_, dst_)

        # get all nodes in the graph except source and destination
        nodes = np.array(H.nodes)
        nodes = nodes.tolist()
        vaild_nodes = [n for n in nodes if n != src_ and n != dst_]
        
        # create origin incidence matrix: columns are all nodes except source and destination, rows are paths
        indices_origin = np.zeros([len(paths), len(vaild_nodes)], dtype=bool)
        indices_columns_origin = vaild_nodes

        # create a dictionary to map node to column index
        node_to_col = {node: i for i, node in enumerate(vaild_nodes)}

        # create incidence matrix
        '''
        for example: 
        src = 1, dst = 4, nodes = [1, 2, 3, 4]
        path1 = [1, 2, 3, 4], path2 = [1, 3, 4]

        indices_origin:
                 2     3  
        path1: true  true
        path2: false true
        '''
        for x in range(len(paths)):
            for comp in vaild_nodes:
                if comp in paths[x]:
                    indices_origin[x, vaild_nodes.index(comp)] = True

        # set the current incidence matrix to origin incidence matrix, update the column names
        indices_current = np.copy(indices_origin)
        indices_columns_current = indices_columns_origin

        # find all true columns index
        all_true_index = np.all(indices_current, axis=0)

        # find all true columns name
        all_true = [indices_columns_current[j] for j, is_true in enumerate(all_true_index) if is_true]

        # find the first pairs that are columns not all true 
        firstpairs = [k for k in indices_columns_current if k not in all_true]

        # loop for finding minimal cut sets
        for k in range(1, order + 1):

            # break if there is no columns in the incidence matrix
            if indices_current.shape[1] == 0:
                break
            
            # find all true columns index
            all_true_index = np.all(indices_current, axis=0)
            all_true = [indices_current_col[j] for j, is_true in enumerate(all_true_index) if is_true]

            # add all true columns to minimal cut sets
            for elem in all_true:
                minimal.append(elem)
            
            if k >= order:
                continue
            
            # find all combinations of k + 1 pairs
            all_combinations = set(combinations(firstpairs, k + 1))

            # put all first pairs into a set
            firstpairsets = set(firstpairs)

            # create a set to store the upper sets
            upperset = set()
            
            # finding all the upper sets of the current minimal cut sets
            for tup in minimal:

                # put the current minimal cut set into a set
                tupset = set(tup)
                # find the remainder of the first pairs and the current minimal cut set
                remainder = firstpairsets - tupset
                # find the number of nodes needed to add to the current minimal cut set
                need = k + 1 - len(tup)

                # if the number of nodes needed is 0, continue
                if need == 0:
                    continue
                
                # find all the combinations of the remainder nodes
                for c in combinations(remainder, need):

                    # add the current minimal cut set and the new combinations of remainder
                    # add the sorted tuple to the upper set
                    upperset.add(tuple(sorted(c + tup)))
            
            # find the new pairs by removing the upper sets of current minimal cut sets from all combinations
            newpairs = list(all_combinations - upperset)

            # create the new incidence matrix, columns are new pairs, rows are paths
            indices_current = np.zeros([len(paths), len(newpairs)], dtype=bool)
            indices_current_col = newpairs
            
            # set the value of the new incidence matrix according to the original incidence matrix for each new pair
            for j, nodes in enumerate(newpairs):
                
                # find the columns index in the original incidence matrix of each node in new pair
                '''
                for example:
                nodes = [2, 3]
                node_to_col = {2: 0, 3: 1}
                indices_tmp = [0, 1]
                '''
                indices_tmp = [node_to_col[node] for node in nodes]

                # select all the columns in the original incidence matrix,
                # do the logical or operation for each row,
                # set the result to the new incidence matrix column
                '''
                for example:
                indices_current:
                        [2, 3]  =    2   OR  3
                path1:   true   =  true  OR true
                path2:   true   =  false OR true
                '''
                indices_current[:, j] = np.any(indices_origin[:, indices_tmp], axis=1)
    
    # convert the minimal cut sets to list of lists
    minimal = [list(i) for i in minimal]

    # sort the minimal cut sets by length and lexicographically
    minimal = sorted(minimal, key=lambda x: (len(x), x))

    # add source and destination to the minimal cut sets
    minimal.insert(0, [dst_])
    minimal.insert(0, [src_])
    
    return minimal
