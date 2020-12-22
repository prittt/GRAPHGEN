// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_FOREST2DAG_H_
#define GRAPHGEN_FOREST2DAG_H_

#include <iterator>
#include <string>
#include <unordered_map>

#include "forest.h"

// Converts forest of decision trees into poly-rooted-dag
struct Forest2Dag {
	std::unordered_map<BinaryDrag<conact>::node*, std::string> ps_; // pointer -> string
	std::unordered_map<std::string, BinaryDrag<conact>::node*> sp_; // string -> pointer
	LineForestHandler& f_;

	std::string Tree2String(BinaryDrag<conact>::node* n);

	void FindAndLink(BinaryDrag<conact>::node* n);

	Forest2Dag(LineForestHandler& f);
};

#endif // !GRAPHGEN_FOREST2DAG_H_