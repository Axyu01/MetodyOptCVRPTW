#include "rng.h"
#include "MutationOps.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>

using namespace std;
MutationOps::MutationOps() {
  // ctor
}

MutationOps::~MutationOps() {
  // dtor
}
Solution *MutationOps::Swap(Solution *s, int numOfSwaps) {
  Solution *s_out = new Solution(s);
  int size = s->size;
  for (int i = 0; i < numOfSwaps; i++) {
    int g1 = -1;
    int g2 = -1;

    while (g1 == g2) {
      g1 = trand() % size;
      g2 = trand() % size;
    }

    int temp = s_out->Genome[g1];
    s_out->Genome[g1] = s_out->Genome[g2];
    s_out->Genome[g2] = temp;
  }

  return s_out;
}
Solution *MutationOps::Swap(Solution *s) { return Swap(s, 1); }
Solution *MutationOps::Inverse(Solution *s) {
  int size = s->size;
  int g1 = -1;
  int g2 = -1;

  while (g1 == g2) {
    g1 = trand() % size;
    g2 = trand() % size;
  }

  if (g1 > g2) // swap
  {
    int temp = g1;
    g1 = g2;
    g2 = temp;
  }

  Solution *s_out = new Solution(s);
  for (int i = g1; i <= g2; i++) {
    s_out->Genome[i] = s->Genome[g2 - (i - g1)];
  }
  s_out->Genome[g2] = s->Genome[g1];

  return s_out;
}
Solution **MutationOps::FindTracks(Problem *problem, Solution *s) {
  Solution **tracks = new Solution *[problem->MAX_VEHICLES];
  int track_id = 0;
  int track_start = 0;
  for (int i = 0; i < problem->MAX_VEHICLES; i++)
    tracks[i] = nullptr;

  int demand = 0;
  for (int g = 0; g < s->size && track_id < problem->MAX_VEHICLES - 1; g++) {
    int to_location = s->Genome[g] + 1;
    if (to_location > problem->SIZE) {
      to_location = 0;
    }
    demand += problem->Demand[to_location];
    if (to_location == 0 && track_start == g) {
      track_start++;
      continue;
    }
    if (demand > problem->CAPACITY || to_location == 0) {
      // Add track
      tracks[track_id] = new Solution(g - track_start);
      for (int l = track_start; l < g; l++) {
        tracks[track_id]->Genome[l - track_start] = s->Genome[l];
      }
      track_id++;
      // reset demand
      demand = problem->Demand[to_location];
      // skip
      track_start = g;
      g--;
    }
  }
  // Add last track and prune it from return symbols, all were used beforehand
  // :(
  tracks[track_id] = new Solution(s->size - track_start);
  int detectedSymbols = 0;
  for (int l = track_start; l < s->size; l++) {
    if (s->Genome[l] >= problem->SIZE) // symbol detected!!
    {
      detectedSymbols++;
      continue;
    }
    tracks[track_id]->Genome[l - track_start - detectedSymbols] = s->Genome[l];
  }
  if (detectedSymbols > 0) {
    Solution *temp = tracks[track_id];
    int finalSize = temp->size - detectedSymbols;
    if (finalSize > 0) {
      tracks[track_id] = new Solution(temp, finalSize);
      track_id++;
    } else {
      tracks[track_id] = nullptr; // last segment was all return symbols, skip it
    }
    delete temp;
  }
  return tracks;
}
Solution *MutationOps::ReconstructSolutionFromTracks(Problem *problem,
                                                     Solution **tracks) {
  // Put things together back
  Solution *sOptimized = new Solution(problem->PREFFERED_GENOME_SIZE);

  int track_id = 0;
  int track_start = 0;
  int RETURN_SYMBOL = problem->SIZE;
  for (int i = 0; i < problem->MAX_VEHICLES; i++) {
    if (tracks[i] == nullptr)
      break;
    for (int g = 0; g < tracks[i]->size; g++) {
      sOptimized->Genome[g + track_start] = tracks[i]->Genome[g];
    }
    if (track_start + tracks[i]->size < sOptimized->size)
      sOptimized->Genome[track_start + tracks[i]->size] =
          RETURN_SYMBOL++; // return symbol
    track_start += tracks[i]->size + 1;
  }
  // Fill rest with zero's
  for (int i = track_start; i < sOptimized->size; i++) {
    sOptimized->Genome[i] = i;
  }
  return sOptimized;
}
Solution *MutationOps::OptimizeTracks(Problem *problem, Solution *s,
                                      int OPT_TRACK_MAX_LOCATION_COUNT) {
  Solution **tracks = FindTracks(problem, s);

  // Optimize tracks
  for (int i = 0; i < problem->MAX_VEHICLES; i++) {
    if (tracks[i] == nullptr)
      break;

    Solution *temp = tracks[i];
    if (tracks[i]->size <= OPT_TRACK_MAX_LOCATION_COUNT) {
      tracks[i] = OptimalTrack(problem, tracks[i]);
    } else {
      Solution *gready = GreadyTrack(problem, tracks[i]);
      problem->EstimateSolution(gready);
      problem->EstimateSolution(tracks[i]);
      if (tracks[i]->eval < gready->eval) {
        temp = gready; // will delete gready this way
      } else {
        tracks[i] = gready; // will save gready if it is better
      }
    }
    delete temp;
  }

  Solution *sOptimized = ReconstructSolutionFromTracks(problem, tracks);

#ifdef DEBUG
  DEBUG_CheckSolution(problem, sOptimized, s, tracks);
#endif

  // Memory management
  for (int i = 0; i < problem->MAX_VEHICLES; i++) {
    if (tracks[i] == nullptr)
      break;
    delete tracks[i];
  }
  delete[] tracks;

  return sOptimized;
}
Solution *MutationOps::GreadyTrack(Problem *problem, Solution *s) {
  bool takenLocation[s->size];
  for (int i = 0; i < s->size; i++)
    takenLocation[i] = false;

  Solution *best = new Solution(0);
  Solution *sLocal = new Solution(s->size); // reused buffer, avoids k² allocations
  for (int i = 0; i < s->size; i++) {
    // Copy the current best prefix into sLocal once per position
    sLocal->size = i + 1;
    for (int k = 0; k < i; k++)
      sLocal->Genome[k] = best->Genome[k];

    double curr_best_eval = 1e18;
    int curr_best_index = -1;
    for (int j = 0; j < s->size; j++) {
      if (takenLocation[j])
        continue;
      sLocal->Genome[i] = s->Genome[j];
      problem->EstimateSolution(sLocal);
      if (sLocal->eval < curr_best_eval) {
        curr_best_eval  = sLocal->eval;
        curr_best_index = j;
      }
    }
    delete best;
    best = new Solution(sLocal, i + 1);
    best->Genome[i] = s->Genome[curr_best_index];
    best->eval = curr_best_eval;
    takenLocation[curr_best_index] = true;
  }
  delete sLocal;
  return best;
}
Solution *MutationOps::OptimalTrack(Problem *problem, Solution *s) {
  int lSize = s->size;
  int *possibleLocations = new int[lSize];
  for (int i = 0; i < lSize; i++) {
    possibleLocations[i] = s->Genome[i];
  }
  Solution *zeroSolution = new Solution(0);
  Solution *best =
      OptimalTrack(problem, zeroSolution, possibleLocations, lSize);
  delete zeroSolution;
  delete[] possibleLocations;
  return best;
}
Solution *MutationOps::OptimalTrack(Problem *problem, Solution *s,
                                    int *possibleLocations, int lSize) {
  if (lSize == 1) {
    Solution *best = new Solution(s, s->size + 1);
    best->Genome[s->size] = possibleLocations[0];
    problem->EstimateSolution(best);
    return best;
  }

  Solution *best = nullptr;
  for (int g = 0; g < lSize; g++) {
    Solution *checked = new Solution(s, s->size + 1);
    checked->Genome[s->size] = possibleLocations[g];
    int checkedLSize = lSize - 1;
    int *checkedPossibleLocations = new int[checkedLSize];
    for (int i = 0; i < checkedLSize; i++) {
      if (i < g)
        checkedPossibleLocations[i] = possibleLocations[i];
      else
        checkedPossibleLocations[i] = possibleLocations[i + 1];
    }
    Solution *sFound =
        OptimalTrack(problem, checked, checkedPossibleLocations, checkedLSize);
    delete checked;
    delete[] checkedPossibleLocations;
    if (best == nullptr || sFound->eval < best->eval) {
      if (best != nullptr)
        delete best;
      best = sFound;
    } else
      delete sFound;
  }
  return best;
}
Solution *MutationOps::Repair(Problem *problem, Solution *s, bool TRY_BEFORE,
                              bool TRY_AFTER, bool USE_DAMAND,
                              int OPT_TRACK_MAX_LOCATION_COUNT) {
  const int BIG_NUMBER = 1000000000;
  // int x;
  // cin>>x;

  Solution **tracks = FindTracks(problem, s);
  Solution **prunedTracks = new Solution *[problem->MAX_VEHICLES];
  int *prunedDemand = new int[problem->MAX_VEHICLES];
  for (int i = 0; i < problem->MAX_VEHICLES; i++) {
    prunedTracks[i] = nullptr;
    if (USE_DAMAND)
      prunedDemand[i] = 0;
    else
      prunedDemand[i] = -BIG_NUMBER;
  }
  int prunedTracksCount = -1;
  // find how many pruned tracks are there to be made
  for (int i = 0; i <= problem->MAX_VEHICLES; i++) {
    Solution *track = nullptr;
    if (i != problem->MAX_VEHICLES)
      track = tracks[i];

    if (track == nullptr) {
      prunedTracksCount = i;
      break;
    }
  }
  // cout << endl<< "DYNAMIC";
  // init and calculate recyclable genes dynamic table + init prunedTracks +
  // find available genes
  double **isGeneRecyclable = new double *[problem->MAX_VEHICLES];
  list<int> availableRecyclableGenes;
  for (int i = 0; i < prunedTracksCount; i++) {
    // cout << endl<<"i"<<i;
    Solution *track = tracks[i];

    isGeneRecyclable[i] = FindRecyclableGenes(problem, track);
    int recyclableGenesCount = 0;
    for (int g = 0; g < track->size; g++) {
      // cout << endl<<"g"<<g;
      if (isGeneRecyclable[i][g] < 0) {
        // cout << endl<<"push";
        availableRecyclableGenes.push_back(track->Genome[g]);
        // cout << endl<<"post push";
        recyclableGenesCount++;
      }
    }
    // cout << endl<<"new"<<track->size<<" - "<<recyclableGenesCount;
    prunedTracks[i] = new Solution(track->size - recyclableGenesCount);
    // cout << endl<<"post new";
  }
  // copy values to prunedTracks + estimate them + init demands
  for (int i = 0; i < prunedTracksCount; i++) {
    Solution *track = tracks[i];
    int prunedIterator = 0;
    for (int g = 0; g < track->size; g++) {
      if (isGeneRecyclable[i][g] >= 0) {
        prunedTracks[i]->Genome[prunedIterator++] = track->Genome[g];
        int town = track->Genome[g] + 1;
        if (town >= problem->SIZE)
          town = 0;
        prunedDemand[i] += problem->Demand[town];
      }
    }
    problem->EstimateSolution(prunedTracks[i]);
  }
  // cout << endl<< "REPAIR";
  //
  // try repairing the solutions
  for (int i = 0; i < prunedTracksCount && availableRecyclableGenes.size() > 0;
       i++) {
    // cout << endl<< "i:"<<i;
    Solution *repairedTrack = tracks[i];
    int prunedIterator = 0;
    bool tryBefore = TRY_BEFORE;
    bool tryAfter = TRY_AFTER;
    int load = 0;
    for (int repairedG = 0; repairedG <= repairedTrack->size &&
                            availableRecyclableGenes.size() > 0;
         repairedG++) {
      // cout << endl<< "repairedG:"<<repairedG;
      if (tryBefore == true) {
        tryBefore = false;
      } else if (repairedG == repairedTrack->size) {
        if (tryAfter == false)
          break;
        tryAfter = false;
      } else if (isGeneRecyclable[i][repairedG] >= 0) {
        prunedIterator++;
        continue;
      }
      // cout << " IS FAULTY";

      int bestGene = 0;
      while (bestGene != -1) {
        // cout<<endl<<"while...";
        bestGene = -1;
        auto bestIt = availableRecyclableGenes.end();

        double bestPenalty = prunedTracks[i]->penalty;
        if (bestPenalty > 0) {
          // cout<<endl<<"BEST PENALTY>0!!!"<<endl;
        }
        // Find posible donor gene
        for (auto it = availableRecyclableGenes.begin();
             it != availableRecyclableGenes.end(); ++it) {
          int gene = *it;
          int town = gene + 1;
          if (town >= problem->SIZE)
            town = 0;
          int demand = problem->Demand[town];
          Solution *copyS = new Solution(prunedTracks[i]);
          copyS->Insert(gene, prunedIterator);
          problem->EstimateSolution(copyS);
          if (copyS->penalty <= bestPenalty &&
              (prunedDemand[i] + demand) <= problem->CAPACITY) {
            bestGene = gene;
            bestIt = it;
            // cout<<endl<<"BEST GENE:"<<bestGene;
            // Replace pruned track
            delete prunedTracks[i];
            prunedTracks[i] = copyS;
            prunedIterator++;
            prunedDemand[i] += demand;
            break;
          } else {
            delete copyS;
          }
        }
        if (bestGene != -1) {
          // cout<<endl<<"erese";
          availableRecyclableGenes.erase(bestIt);
          // cout<<endl<<"post erese";
        }
      }
      // cout<<endl<<"post while...";
    }
  }
  // cout << endl<< "LEFTOVER";
  // Use leftover values gene values
  if (availableRecyclableGenes.size() > 0) {
    if (prunedTracksCount < problem->MAX_VEHICLES) {
      // cout <<endl<<"recycled";
      Solution *newTrack = new Solution((int)availableRecyclableGenes.size());
      int i = 0;
      for (auto it = availableRecyclableGenes.begin();
           it != availableRecyclableGenes.end(); ++it) {
        newTrack->Genome[i++] = *it;
      }
      prunedTracks[prunedTracksCount++] = newTrack;
    } else {
      // cout <<endl<<"copy + recycled";
      Solution *lastSolution = prunedTracks[prunedTracksCount - 1];
      Solution *newTrack =
          new Solution((int)availableRecyclableGenes.size() + lastSolution->size);
      int i = 0;
      while (i < lastSolution->size) {
        newTrack->Genome[i] = lastSolution->Genome[i];
        i++;
      }
      for (auto it = availableRecyclableGenes.begin();
           it != availableRecyclableGenes.end(); ++it) {
        newTrack->Genome[i++] = *it;
      }
      prunedTracks[prunedTracksCount - 1] = newTrack;
      delete lastSolution;
    }
  }
  // cout<<endl<<"OPTIMIZE LAST TRACK "<< prunedTracksCount<<endl;
  // prunedTracks[prunedTracksCount-1]->print();
  // Optimize last pruned track
  if (prunedTracks[prunedTracksCount - 1]->size >
      OPT_TRACK_MAX_LOCATION_COUNT) {
    Solution *temp = prunedTracks[prunedTracksCount - 1];
    if (temp->size <= OPT_TRACK_MAX_LOCATION_COUNT) {
      prunedTracks[prunedTracksCount - 1] = OptimalTrack(problem, temp);
    } else {
      Solution *gready = GreadyTrack(problem, temp);
      problem->EstimateSolution(gready);
      problem->EstimateSolution(temp);
      if (temp->eval < gready->eval) {
        temp = gready; // will delete gready this way
      } else {
        prunedTracks[prunedTracksCount - 1] =
            gready; // will save gready if it is better
      }
    }
    delete temp;
  }
  // recycledSolution->print();
  // Solution* wellRecycledSolution =
  // Repair(problem,recycledSolution,correctedGenes);//The solution is well
  // recycled, return it delete recycledSolution; cout <<endl<<"NORMAL";
  // DEBUG_PRINT_Tracks(problem,tracks);
  // cout <<endl<<"PRUNED";
  // DEBUG_PRINT_Tracks(problem,prunedTracks);

  // cout <<endl<<"REPAIR";

  Solution *sRepaired = ReconstructSolutionFromTracks(problem, prunedTracks);
  problem->EstimateSolution(sRepaired);
  // cout <<endl<<"POST REPAIR";

#ifdef DEBUG
  DEBUG_CheckSolution(problem, sRepaired, s, tracks);
#endif
  // cout <<endl<<"MEMORY MANAGMENT";
  // Memory management
  for (int i = 0; i < prunedTracksCount; i++) {
    // cout <<endl<<"i"<<i;
    Solution *track = tracks[i];
    delete prunedTracks[i];
    if (track == nullptr)
      continue;
    // cout <<endl<<"i"<<i;
    delete track;
    delete[] isGeneRecyclable[i];
  }
  // cout <<endl<<"del recyc";
  delete[] isGeneRecyclable;
  // cout <<endl<<"del tracks";
  delete[] tracks;
  // cout <<endl<<"del tracks pruned";
  delete[] prunedTracks;
  delete[] prunedDemand;
  // cout <<endl<<"return";

  return sRepaired;
}
double *MutationOps::FindRecyclableGenes(Problem *problem, Solution *s) {
  int SIZE = problem->SIZE;
  int CAPACITY = problem->CAPACITY;
  double *isGeneRecyclable = new double[s->size];
  for (int i = 0; i < s->size; i++) {
    isGeneRecyclable[i] =
        0; // If value is 0 or greater,This means the gene is not recyclable
  }

  double estimation = 0;
  int load = 0;
  double time = 0;
  int currentNode = 0;
  int previousNode = 0;

  for (int i = 0; i < s->size; i++) {
    currentNode = s->Genome[i] + 1;
    int demand = 0;
    if (currentNode > SIZE)
      currentNode = 0;

    if (currentNode == 0) {
      load = 0;
      time = 0;
      if (previousNode == 0) {
        isGeneRecyclable[i] = -1; // This means the gene is suited to be
                                  // recycled
        continue;
      }
    } else
      demand = problem->Demand[currentNode];

    if (load + demand > CAPACITY) {
      estimation += problem->DistanceMatrix[previousNode][0];
      load = 0;
      time = 0;
      previousNode = 0;
      i--;
      continue;
    }

    int timeDelta = problem->DistanceMatrix[previousNode][currentNode];

    // check for time window
    if (time + timeDelta >
        problem->DueDate[currentNode]) // if late, add gene to recycle and
                                       // rollback to previous location
    {
      isGeneRecyclable[i] = -1; // This means the gene is suited to be recycled
      continue;
    }
    estimation += problem->DistanceMatrix[previousNode][currentNode];
    time += timeDelta;
    load += demand;
    if (time < problem->ReadyTime[currentNode]) {
      time = problem->ReadyTime[currentNode];
    }
    time += problem->ServiceTime[currentNode];
    isGeneRecyclable[i] = time; // This means the gene is NOT suited to be
                                // recycled + gives info about departure time

    if (i > 0 && isGeneRecyclable[i - 1] <
        0) // If previous gene is recyclable update info of recyclable chain
    {
      int recyclableTown = i - 1;
      while (recyclableTown > 0 &&
             isGeneRecyclable[recyclableTown] < 0) // update recyclable chain
      {
        isGeneRecyclable[recyclableTown] =
            -time; // give info about upper constraint for possible future
                   // replacements
        recyclableTown--;
      }
    }

    previousNode = currentNode;
  }
  estimation += problem->DistanceMatrix[currentNode][0];

  // Mark all last return genes as not recyclable
  /*for(int g=s->size-1;g>=0;g--)
  {
      if(s->Genome[g]+1 > SIZE)//found return base token
          isGeneRecyclable[g] = false;
      else
          break;
  }*/

  return isGeneRecyclable;
}
Solution *MutationOps::RedistributeLocations(Problem *problem, Solution *s,
                                             int tries) {
  Solution **tracks = FindTracks(problem, s);
  int *demand = new int[problem->MAX_VEHICLES];
  for (int i = 0; i < problem->MAX_VEHICLES; i++) {
    demand[i] = 0;
  }
  int tracksCount = -1;
  // find how many tracks are there
  for (int i = 0; i <= problem->MAX_VEHICLES; i++) {
    Solution *track = nullptr;
    if (i != problem->MAX_VEHICLES)
      track = tracks[i];

    if (track == nullptr) {
      tracksCount = i;
      break;
    }
  }
  if (tracksCount == -1)
    return new Solution(s);
  if (tracksCount <= 1) {
    for (int i = 0; i < tracksCount; i++)
      delete tracks[i];
    delete[] tracks;
    delete[] demand;
    return new Solution(s);
  }
  for (int i = 0; i < tries; i++) {
    int donor = trand() % tracksCount;
    if (tracks[donor]->size <= 1)
      continue;
    int pacient = trand() % tracksCount;
    while (donor == pacient) {
      pacient = trand() % tracksCount;
    }

    if (tracks[donor]->size < tracks[pacient]->size) {
      int temp = donor;
      donor = pacient;
      pacient = temp;
    }
    int geneRemovePos = trand() % tracks[donor]->size;
    int geneVal = tracks[donor]->Genome[geneRemovePos];
    int geneInsertPos = trand() % (tracks[pacient]->size + 1);
    tracks[donor]->Remove(geneRemovePos);
    tracks[pacient]->Insert(geneVal, geneInsertPos);
  }

  Solution *sRedistributed = ReconstructSolutionFromTracks(problem, tracks);
  // Manage memory
  for (int i = 0; i < tracksCount; i++)
    delete tracks[i];
  delete[] tracks;
  delete[] demand;

  return sRedistributed;
}
void MutationOps::DEBUG_CheckSolution(Problem *problem, Solution *checked,
                                      Solution *original, Solution **tracks) {
  // check for debugging purposes
  bool foundError = false;
  int foundErrorID = -1;
  for (int i = 0; i < checked->size; i++) {
    if (foundError)
      break;
    foundError = true;
    foundErrorID = i;
    for (int j = 0; j < checked->size; j++) {
      if (checked->Genome[j] == i) {
        foundError = false;
        break;
      }
    }
  }
  if (foundError) {
    cout << endl << "[repair] broken solution: missing gene " << foundErrorID;
    checked->print();
    cout << endl << "[repair] original solution:";
    original->print();
    cout << endl << "[repair] tracks used for reconstruction:";
    for (int t = 0; t < problem->MAX_VEHICLES; t++) {
      if (tracks[t] == nullptr)
        break;
      tracks[t]->print();
    }
    cout << endl << "[repair] continuing" << endl;
  }
}
void MutationOps::DEBUG_PRINT_Tracks(Problem *problem, Solution **tracks) {
  cout << endl << "[tracks] current split:";
  int numOfTracks = problem->MAX_VEHICLES;
  for (int t = 0; t < problem->MAX_VEHICLES; t++) {
    if (tracks[t] == nullptr) {
      numOfTracks = t;
      break;
    }
    tracks[t]->print();
  }
  cout << endl << "[tracks] count: " << numOfTracks;
}
