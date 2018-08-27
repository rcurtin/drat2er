#ifndef DRAT2ER_RAT_ELIMINATOR_H_
#define DRAT2ER_RAT_ELIMINATOR_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "proof_transformer.h"

namespace drat2er
{

class Formula;
class Clause;
class RupClause;
class RatClause;

// RatEliminator performs the most important step of the transformation from 
// DRAT to extended resolution: It takes the input proof (in the LRAT format) 
// and replaces all proper RAT additions by a definition clauses and
// RUP additions. It also eliminates all deletion steps from the DRAT proof.
//
// This transformation follows the transformation as described in
// our paper with the following two exceptions: 
//
// 1. In the paper, we transform a single proper RAT addition into a 
// sequence of definition clauses, RUPs, and deletions - later we then also
// eliminate the deletion steps. Here, we immediately eliminate the deletion
// steps by not writing them to the output. This is sound and more efficient.
// 
// 2. In the paper, after transforming a single RAT addition, we rename the
// replaced variable (in the paper, we replace p by x), in all
// subsequent instructions in the proof. Here, we do not
// perform this renaming explicitly by changing the proof file. 
// Instead, we maintain a mapping that "encodes" the renaming: 
// Whenever we encounter a new instruction in the proof, we first apply this 
// mapping and then process the instruction. This improves the performance. 
class RatEliminator : public ProofTransformer
{
 public:
  RatEliminator(std::shared_ptr<Formula> formula, 
                int max_variable,
                int max_instruction,
                bool print_progress=false);

  virtual void HandleProperRatAddition(const RatClause& rat) override;
  virtual void HandleRupAddition(const RupClause& rup) override;
  virtual void HandleDeletion(const Deletion& deletion) override;

  // Performs the real meat of the transformation: Takes a RAT clause
  // and transforms it into a sequence of definition clauses and RUPs. 
  void ReplaceByDefinitionRUPsAndDeletions(const RatClause& rat);

  // Takes a RAT clause and a variable and returns the definition clauses
  // corresponding to this RAT clause as described in step 1 within section 4.1
  // of our paper.
  std::vector<Clause> CorrespondingDefinition(const RatClause& rat, 
                                              const int new_variable);

  // Returns the first definition clause corresponding to a given RAT:
  // Given a clause p | c_1 | ... | c_k and a new variable x, this function
  // returns the clause x | c_1 | ... | c_k.
  Clause FirstDefinitionClause(const RatClause& rat, 
                               const int new_variable) const;


  // Returns the first definition clause corresponding to a given RAT:
  // Given a clause p | c_1 | ... | c_k and a new variable x, this function
  // returns the clause x | -p.
  Clause SecondDefinitionClause(const RatClause& rat, 
                                const int new_variable);

  // Returns the third block of definition clauses corresponding to a given
  // RAT: Given a clause p | c_1 | ... | c_k and a new variable x, this 
  // function returns the clauses {-x | p | -c_1, ..., -x | p | -c_k}.
  std::vector<Clause> ThirdBlockOfDefinitionClauses(const RatClause& rat,
                                              const int new_variable);
  template<typename T>
  T RenameLiterals(const T& clause) const;
  RupClause RenameRup(const RupClause& clause) const;
  RatClause RenameRat(const RatClause& clause) const;
  Deletion RenameDeletion(const Deletion& deletion) const;
  int RenameClauseIndex(const int clause_index) const;
  int RenameLiteral(const int literal) const;
  void UpdateLiteralRenaming(const int old_literal, const int new_literal);
  void UpdateClauseRenaming(const int old_index, const int new_index);

 private:
  int AddDefinitionsForRatClause(const RatClause& clause);
  void ReplaceOldPivotByNew(const RatClause& rat, 
                            const std::vector<Clause>& definition);
  void ReplacePositivePivot(const RatClause& rat,
                            const std::vector<Clause>& definition);  
  void ReplaceNegativePivot(const RatClause& rat,
                            const std::vector<Clause>& definition);  
  void DeleteClausesWithOldVariable(const int old_variable,
                                    const std::vector<Clause>& definition);

  void WriteDefinitionToOutput(const std::vector<Clause>& definition);
  void WriteDeletionToOutput(const Deletion& deletion);
  void WriteDeletionToOutput(const std::vector<Clause>& clauses,
                             int instruction_index);
  static void UpdateMapping(std::unordered_map<int,int>& mapping,
                            std::unordered_map<int,int>& inverse_mapping, 
                            const int old_value, const int new_value);

  std::shared_ptr<Formula> formula_;
  int max_variable_;
  int max_instruction_;
  std::unordered_map<int,int> old_to_new_literal_;
  std::unordered_map<int,int> new_to_old_literal_;
  std::unordered_map<int,int> old_to_new_clause_;
  std::unordered_map<int,int> new_to_old_clause_;
};

}// namespace

#endif
