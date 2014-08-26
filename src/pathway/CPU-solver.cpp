#include "pathway/CPU-solver.hpp"

struct node_t {
    int id;
    float dist;
    node_t *prev;
    node_t() = default;
    node_t(int id, float dist, node_t *prev)
        : id(id), dist(dist), prev(prev) { }
};

CPUPathwaySolver::CPUPathwaySolver(Pathway *pathway)
    : p(pathway)
{
    // pass
}

CPUPathwaySolver::~CPUPathwaySolver()
{
    for (auto &pair : globalList)
        delete pair.second;
    globalList.clear();
}

void CPUPathwaySolver::initialize()
{
    for (auto &pair : globalList)
        delete pair.second;
    globalList.clear();

    openList = decltype(openList)();
    closeList.clear();

    target = vec2(p->ex(), p->ey());
    targetID = p->toID(p->ex(), p->ey());

    int startID = p->toID(p->sx(), p->sy());
    node_t *startNode = new node_t(startID, 0, nullptr);
    openList.push(make_pair(computeFValue(startNode), startNode));
    globalList[startNode->id] = startNode;
}

bool CPUPathwaySolver::solve(float *optimal, vector<vec2> *solution)
{
    while (!openList.empty()) {
        node_t *node;
        do {
            node = openList.top().second;
            openList.pop();
        } while (closeList.count(node->id));
        closeList.insert(node->id);

        dout << p->toVec(node->id) << endl;

        if (node->id == targetID) {
            *optimal = node->dist;
            solution->clear();
            while (node) {
                solution->push_back(p->toVec(node->id));
                node = node->prev;
            }
            std::reverse(solution->begin(), solution->end());
            return true;
        }

        int x, y;
        p->toXY(node->id, &x, &y);
        for (int i = 0; i < 8; ++i) {
            if (~p->graph()[node->id] & 1 << i)
                continue;
            int nx = x + DX[i];
            int ny = y + DY[i];
            if (p->inrange(nx, ny)) {
                int nid = p->toID(nx, ny);
                float dist = node->dist + COST[i];
                if (globalList.count(nid) == 0) {
                    node_t *nnode = new node_t(nid, dist, node);
                    globalList[nid] = nnode;
                    openList.push(make_pair(computeFValue(nnode), nnode));
                    dout << '\t' << vec2(nx, ny) << " n " << computeFValue(nnode) << endl;
                } else {
                    node_t *onode = globalList[nid];
                    if (dist < onode->dist) {
                        onode->dist = dist;
                        openList.push(make_pair(computeFValue(onode), onode));
                        dout << '\t' << vec2(nx, ny) << " u " << computeFValue(onode) << endl;
                    }
                }
            }
        }
    }
    return false;
}

float CPUPathwaySolver::computeFValue(node_t *node)
{
    int x, y;
    p->toXY(node->id, &x, &y);
    int dx = abs(x - p->ex());
    int dy = abs(y - p->ey());
    return node->dist + min(dx, dy)*SQRT2 + abs(dx-dy);
}
