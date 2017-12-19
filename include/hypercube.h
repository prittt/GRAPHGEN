#pragma once

#include "rule_set.h"
#include <iostream>
#include "cassert"

typedef unsigned char byte;

enum VDim :byte { Zero = 0, One = 1, Indifference = 2 };

struct VIndex {
	int m_iDim;
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
		for (int i = 0; i<m_iDim; i++) {
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
		for (int i = 0; i<m_iDim; i++) {
			s[i] = aRepresentation[m_arrIndex[i]];
		}
		return s;
	}

	unsigned GetIndex() const {
		unsigned ui(0);
		for (int i = 0; i<m_iDim; i++) {
			ui *= 3;
			ui += m_arrIndex[i];
		}
		return ui;
	}

	bool MoveNext() {
		for (int i = m_iDim - 1; i >= 0; i--) {
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

#pragma pack(1)
struct VNode {
	unsigned uiAction;
	/*unsigned*/ unsigned long long uiProb;
    /*unsigned*/ unsigned long long uiGain;
	byte uiMaxGainIndex;

	VNode() : uiAction(0), uiProb(0), uiGain(0), uiMaxGainIndex(0) {
	}
};
#pragma pack(8)

template <typename T>
std::istream& rawread(std::istream& is, T& val, size_t n = sizeof(val)) {
    return is.read(reinterpret_cast<char*>(&val), n);
}
template <typename T>
std::ostream& rawwrite(std::ostream& os, const T& val, size_t n = sizeof(val)) {
    return os.write(reinterpret_cast<const char*>(&val), n);
}

struct VHyperCube {
	size_t m_iDim;
	std::vector<VNode> m_arrIndex;

	VHyperCube(size_t iDim) : m_iDim(iDim), m_arrIndex(unsigned(pow(3.0, iDim))) {
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

	void initialize_rules(const rule_set& rs) {
		auto nvars = rs.conditions.size();
		auto nrules = rs.rules.size();
        
        assert(nvars == m_iDim);

		for (size_t i = 0; i < nrules; ++i) {
            std::string s = binary(i, nvars);
            std::reverse(begin(s), end(s));
			VIndex idx(s);
			m_arrIndex[idx.GetIndex()].uiProb = rs.rules[i].frequency;
			m_arrIndex[idx.GetIndex()].uiAction = rs.rules[i].actions;
		}
	}

	void optimize(bool bVerbose = false) {
		std::string s;
		s.resize(m_iDim, '0');
		VIndex idx(s);
		if (bVerbose) {
			// Stampo la tabella
			do {
				std::cout << idx.GetIndexString() << "\t" << m_arrIndex[idx.GetIndex()].uiProb << "\t";
				if (m_arrIndex[idx.GetIndex()].uiAction == 0)
					std::cout << "0";
				else
					for (unsigned i = 1; i<32; i++)
						if (m_arrIndex[idx.GetIndex()].uiAction & (1 << (i - 1)))
							std::cout << i << ",";
				std::cout << "\n";
			} while (idx.MoveNext());
			std::cout << "------------------------\n";
		}

		for (size_t iNumIndifference = 1; iNumIndifference <= m_iDim; iNumIndifference++) {
			if (!bVerbose)
				std::cout << iNumIndifference << " ";
			// Inizializzo le indifferenze
			bool bFine;
			std::vector<size_t> arrPosIndifference(iNumIndifference);
			int iPos = 0;
			for (size_t i = 0; i<iNumIndifference; i++) {
				arrPosIndifference[i] = iPos;
				iPos++;
			}

			std::vector</*unsigned*/unsigned long long> arrProb(iNumIndifference);
			std::vector</*unsigned*/unsigned long long> arrGain(iNumIndifference);

			// Faccio tutte le combinazioni
			do {
				// Genero la maschera delle indifferenze
				std::string s;
				s.resize(m_iDim, '0');
				for (size_t i = 0; i<iNumIndifference; i++)
					s[arrPosIndifference[i]] = '-';

				// Stampo tutte le combinazioni
				if (!idx.SetIndex(s))
					throw;
				do {
					for (size_t i = 0; i<iNumIndifference; i++) {
						std::string sChild0(idx.GetIndexString());
						std::string sChild1(sChild0);
						sChild0[arrPosIndifference[i]] = '0';
						sChild1[arrPosIndifference[i]] = '1';
						VIndex idx0(sChild0), idx1(sChild1);
						VNode node0(m_arrIndex[idx0.GetIndex()]), node1(m_arrIndex[idx1.GetIndex()]);

						// Faccio l'intersezione delle possibili azioni
						unsigned uiIntersezione = node0.uiAction & node1.uiAction;

						m_arrIndex[idx.GetIndex()].uiAction = uiIntersezione;
						arrProb[i] = node0.uiProb + node1.uiProb;
						arrGain[i] = node0.uiGain + node1.uiGain;

						if (uiIntersezione>0) {
							arrGain[i] += arrProb[i];
						}
					}
					/*unsigned*/unsigned long long uiMaxGain(0);
                    /*unsigned*/unsigned long long uiMaxGainProb(0);
					unsigned uiMaxGainIndex(0);
					for (size_t i = 0; i<iNumIndifference; i++) {
						if (uiMaxGain <= arrGain[i]) {
							uiMaxGain = arrGain[i];
							uiMaxGainProb = arrProb[i];
							uiMaxGainIndex = arrPosIndifference[i];
						}
					}
					m_arrIndex[idx.GetIndex()].uiGain = uiMaxGain;
					m_arrIndex[idx.GetIndex()].uiProb = uiMaxGainProb;
					m_arrIndex[idx.GetIndex()].uiMaxGainIndex = uiMaxGainIndex;

					if (bVerbose) {
						std::cout << idx.GetIndexString() << "\t" << m_arrIndex[idx.GetIndex()].uiProb << "\t";
						if (m_arrIndex[idx.GetIndex()].uiAction == 0)
							std::cout << "0";
						else
							for (unsigned j = 1; j<32; j++)
								if (m_arrIndex[idx.GetIndex()].uiAction & (1 << (j - 1)))
									std::cout << j << ",";
						std::cout << "\t";

						for (size_t i = 0; i<iNumIndifference; i++) {
							std::cout << arrGain[i];
							if (arrPosIndifference[i] == uiMaxGainIndex)
								std::cout << "*";
							else if (arrGain[i] == uiMaxGain)
								std::cout << "#";
							std::cout << "\t";
						}
						std::cout << "\n";
					}
				} while (idx.MoveNext());
				if (bVerbose)
					std::cout << "\n";

				// Passo alla permutazione di indifferenze successiva
				bFine = true;
				for (int i = iNumIndifference - 1; i >= 0; i--) {
					arrPosIndifference[i]++;
					// Ho una posizione valida?
					if (arrPosIndifference[i] < m_iDim) {
						// Ci stanno le altre indifferenze?
						if (m_iDim - 1 - arrPosIndifference[i] >= iNumIndifference - 1 - i) {
							// La posizione è valida, ci stanno le altre, allora sistemo 
							// le indifferenze successive
							iPos = arrPosIndifference[i] + 1;
							for (size_t j = i + 1; j<iNumIndifference; j++) {
								arrPosIndifference[j] = iPos;
								iPos++;
							}
							bFine = false;
							break;
						}
					}
				}
			} while (!bFine);
			if (bVerbose)
				std::cout << "------------------------\n";
		}
	}
};
