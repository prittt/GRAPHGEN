// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_HYPERCUBE_H_
#define GRAPHGEN_HYPERCUBE_H_

#include <algorithm>
#include <cassert>
#include <iostream>

#include "conact_tree.h"
#include "rule_set.h"

typedef unsigned char byte;


enum VDim { Zero = 0, One = 1, Indifference = 2 };

struct VIndex {
	size_t m_iDim;
	std::vector<VDim> m_arrIndex;

	VIndex(int iDim = 0) : m_iDim(iDim), m_arrIndex(iDim) {
	}

	VIndex(const std::string &s) : m_iDim(s.length()), m_arrIndex(s.length()) {
		SetIndex(s);
	}

	void SetDim(int iDim) {
		m_iDim = iDim;
		m_arrIndex.resize(iDim);
	}

	bool SetIndex(const std::string &s) {
		if (s.length() != m_iDim)
			return false;
		for (size_t i = 0; i<m_iDim; i++) {
			if (s[i] == '0')
				m_arrIndex[i] = Zero;
			else if (s[i] == '1')
				m_arrIndex[i] = One;
			else if (s[i] == '-')
				m_arrIndex[i] = Indifference;
			else
				return false;
		}
		return true;
	}

	std::string GetIndexString() const {
		std::string s;
		s.resize(m_iDim);
		char aRepresentation[3] = { '0','1','-' };
		for (size_t i = 0; i<m_iDim; i++) {
			s[i] = aRepresentation[m_arrIndex[i]];
		}
		return s;
	}

	unsigned GetIndex() const {
		unsigned ui(0);
		for (size_t i = 0; i<m_iDim; i++) {
			ui *= 3;
			ui += m_arrIndex[i];
		}
		return ui;
	}

	bool MoveNext() {
		for (int i = int(m_iDim) - 1; i >= 0; i--) {
			if (m_arrIndex[i] == Indifference)
				continue;
			else if (m_arrIndex[i] == Zero) {
				m_arrIndex[i] = One;
				return true;
			}
			else
				m_arrIndex[i] = Zero;
		}
		return false;
	}
};

#pragma pack(push)
#pragma pack(1)
struct VNode {
	std::bitset</*11881*/128> uiAction;
	/*unsigned*/ unsigned long long uiProb;
    /*unsigned*/ unsigned long long uiGain;
	byte uiMaxGainIndex;
    unsigned neq = 1;

	VNode() : uiAction(0), uiProb(0), uiGain(0), uiMaxGainIndex(0) {
	}
};
#pragma pack(pop)

template <typename T>
std::istream& rawread(std::istream& is, T& val, size_t n) {
    return is.read(reinterpret_cast<char*>(&val), n);
}
template <typename T>
std::ostream& rawwrite(std::ostream& os, const T& val, size_t n) {
    return os.write(reinterpret_cast<const char*>(&val), n);
}

struct VHyperCube {
	size_t m_iDim;
	std::vector<VNode> m_arrIndex;
    const rule_set& m_rs;

	VHyperCube(const rule_set& rs) : m_rs(rs), m_iDim(rs.conditions.size()), m_arrIndex(unsigned(pow(3.0, rs.conditions.size()))) {
        // Initialize hypercube nodes using the rules defined in the ruleset
        auto nrules = rs.rules.size();
        for (size_t i = 0; i < nrules; ++i) {
            // for each rule generate the hypercube index
			std::string s = binary(i, m_iDim);
            std::reverse(begin(s), end(s));
            VIndex idx(s);
            // and set its values
            m_arrIndex[idx.GetIndex()].uiProb = rs.rules[i].frequency;
            m_arrIndex[idx.GetIndex()].uiAction = rs.rules[i].actions;
        }
    }

    std::istream& read(std::istream& is) {
        return rawread(is, m_arrIndex[0], m_arrIndex.size() * sizeof(VNode));
    }
    std::ostream& write(std::ostream& os) {
        return rawwrite(os, m_arrIndex[0], m_arrIndex.size() * sizeof(VNode));
    }

	VNode& operator[](const VIndex &idx) {
		return m_arrIndex[idx.GetIndex()];
	}
	const VNode& operator[](const VIndex &idx) const {
		return m_arrIndex[idx.GetIndex()];
	}

    BinaryDrag<conact> optimize(bool bVerbose = false);
};

// Generates an Optimal Decision Tree from the given rule_set,
// and store it in the filename when specified.
BinaryDrag<conact> GenerateOdt(const rule_set& rs);
BinaryDrag<conact> GenerateOdt(const rule_set& rs, const std::string& filename);

/** @brief Returns the optimal (or pseudo optimal) decision tree generated from the given rule set

This function generates the optimal decision tree from the given rule set. When the number
of rules is too high, a pseudo optimal tree is generated. If the tree has already been generated, it
is loaded from file, unless the "force_generation" parameter is set to true. In this case the tree
is always regenerated. The loaded/generated tree is then returned from the function.

@param[in] rs Rule set from which generate the decision tree.
@param[in] force_generation Whether the tree must be generated or can be loaded from file.

@return The optimal decision tree associated to the specified rule set.
*/
BinaryDrag<conact> GetOdt(const rule_set& rs, bool force_generation = false);


/** @brief Returns the optimal (or pseudo optimal) decision tree generated from the given rule set

This function generates the optimal decision tree from the given rule set. When the number
of rules is too high, a pseudo optimal tree is generated. If the tree has already been generated, it
is loaded from file, unless the "force_generation" parameter is set to true. In this case the tree
is always regenerated. The loaded/generated tree is then returned from the function.

@param[in] rs Rule set from which generate the decision tree.
@param[in] file_suffix Suffix that is appended to the file name of the decision tree file.
@param[in] force_generation Whether the tree must be generated or can be loaded from file.

@return The optimal decision tree associated to the specified rule set.
*/
BinaryDrag<conact> GetOdtWithFileSuffix(const rule_set& rs, const std::string& file_suffix, bool force_generation = false);


#endif // !GRAPHGEN_CONACT_TREE_H_