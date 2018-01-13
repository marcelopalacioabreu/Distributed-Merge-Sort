#ifndef SERVER_H
#define SERVER_H
#include<vector>
#include<queue>

bool isSorted(const std::vector<int> &vec);

std::vector<int> kMerge(std::vector<std::queue<int>> &unmerged);

#endif
