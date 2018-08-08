#include "hypercube.h"

#include "utilities.h"

using namespace std;

void CreateTree_rec(ltree& t, ltree::node *n, const rule_set& rs, const VHyperCube &hcube, const VIndex &idx) {
	VNode node = hcube[idx];
	if (node.uiAction == 0) {
		n->data.t = conact::type::CONDITION;
		n->data.condition = rs.conditions[node.uiMaxGainIndex];

		// Estraggo i due (n-1)-cubi
		string sChild0(idx.GetIndexString());
		string sChild1(sChild0);
		sChild0[node.uiMaxGainIndex] = '0';
		sChild1[node.uiMaxGainIndex] = '1';
		VIndex idx0(sChild0), idx1(sChild1);

		CreateTree_rec(t, n->left = t.make_node(), rs, hcube, idx0);
		CreateTree_rec(t, n->right = t.make_node(), rs, hcube, idx1);
	}
	else {
		n->data.t = conact::type::ACTION;
		n->data.action = node.uiAction;
	}
}

ltree VHyperCube::optimize(bool bVerbose)
{
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
				for (unsigned i = 1; i < 32; i++)
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
		for (size_t i = 0; i < iNumIndifference; i++) {
			arrPosIndifference[i] = iPos;
			iPos++;
		}

		std::vector</*unsigned*/unsigned long long> arrProb(iNumIndifference);
		std::vector</*unsigned*/unsigned long long> arrGain(iNumIndifference);
		std::vector<unsigned> arrNEq(iNumIndifference);

		// Faccio tutte le combinazioni
		do {
			// Genero la maschera delle indifferenze
			std::string s;
			s.resize(m_iDim, '0');
			for (size_t i = 0; i < iNumIndifference; i++)
				s[arrPosIndifference[i]] = '-';

			// Stampo tutte le combinazioni
			if (!idx.SetIndex(s))
				throw;
			do {
				for (size_t i = 0; i < iNumIndifference; i++) {
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
					arrNEq[i] = node0.neq * node1.neq;

					if (uiIntersezione > 0) {
						arrGain[i] += arrProb[i];
						arrNEq[i] = 0;
					}
				}
				/*unsigned*/unsigned long long uiMaxGain(0);
				/*unsigned*/unsigned long long uiMaxGainProb(0);
				unsigned uiMaxGainIndex(0);
				unsigned uiNEq(0);
				for (size_t i = 0; i < iNumIndifference; i++) {
					if (uiMaxGain <= arrGain[i]) {
						if (uiMaxGain < arrGain[i])
							uiNEq = arrNEq[i];
						else
							uiNEq += arrNEq[i];
						uiMaxGain = arrGain[i];
						uiMaxGainProb = arrProb[i];
						uiMaxGainIndex = arrPosIndifference[i];
					}
				}
				m_arrIndex[idx.GetIndex()].uiGain = uiMaxGain;
				m_arrIndex[idx.GetIndex()].uiProb = uiMaxGainProb;
				m_arrIndex[idx.GetIndex()].uiMaxGainIndex = uiMaxGainIndex;
				m_arrIndex[idx.GetIndex()].neq = std::max(uiNEq, 1u);

				if (bVerbose) {
					std::cout << idx.GetIndexString() << "\t" << m_arrIndex[idx.GetIndex()].uiProb << "\t";
					if (m_arrIndex[idx.GetIndex()].uiAction == 0)
						std::cout << "0";
					else
						for (unsigned j = 1; j < 32; j++)
							if (m_arrIndex[idx.GetIndex()].uiAction & (1 << (j - 1)))
								std::cout << j << ",";
					std::cout << "\t";

					for (size_t i = 0; i < iNumIndifference; i++) {
						std::cout << arrGain[i];
						if (arrPosIndifference[i] == uiMaxGainIndex)
							std::cout << "*";
						else if (arrGain[i] == uiMaxGain)
							std::cout << "#";
						std::cout << "\t";
					}

					std::cout << m_arrIndex[idx.GetIndex()].neq << "\n";
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
						for (size_t j = i + 1; j < iNumIndifference; j++) {
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

	ltree t;
	CreateTree_rec(t, t.make_root(), m_rs, *this, string(m_iDim, '-'));
	return t;
}

ltree GenerateOdt(const rule_set& rs, const char *filename) {
	LOG("Allocating hypercube",
		VHyperCube hcube(rs);
	);

	LOG("Optimizing rules",
		auto t = hcube.optimize(false);
	);

	ofstream os;
	if (filename) {
		os.open(filename);
		if (os.is_open()) {
			WriteConactTree(t, filename);
		}
	}

	return t;
}
