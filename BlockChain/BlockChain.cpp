#include <iostream>
#include <vector>
#include <limits>
#include <chrono>

using namespace std;
using Satoshi = uint64_t;
using Address = uint64_t;

//The amount of money created every block and given to the block's miner
#define MONEY_CREATED_FOR_THE_MINER_EACH_BLOCK 1000
#define MAX_ENTRIES_IN_BLOCK 10
#define SIG_LENGTH 32

struct Transaction
{
	//creator of transaction and sender of funds
	Address from;
	//transaction destination
	Address to;
	//amount of money to send to the destination
	Satoshi amount;
	//amount of fee is offered by the creator to the miner to add this transaction to the blockchain
	Satoshi fee;
	//creation time, written by the transaction creator
	time_t creationTime;
	//signing {from,amount,to,fee,creation time}
	uint8_t signature[SIG_LENGTH];
};

struct Block
{
	//holds up to 10 transactions
	std::vector<Transaction> trans;
	//the miner that added the block to the blockchain
	Address miner;
	// the next block at the blockchain (is nullptr for the last block)
	Block* next;
};



class Blockchain
{
	//not null
	Block* m_firstBlock;
public:
	//assumption firstBlock is not null
	Blockchain(Block* firstBlock) :m_firstBlock(firstBlock) {}
	//Assumption: all the transaction in the blockchain ("firstBlock linkedlist") are valid
	//return whether or not the new transaction is valid, given this blockchain
	//No need to validate the crypto signature
	bool isValid(const Transaction& newTransaction)const;
};

bool Blockchain::isValid(const Transaction& newTransaction) const
{
	// check if transaction ammount is 0
	if (newTransaction.amount == 0)
		return false;

	// check if the creation time is in the future
	auto curr_time_point = std::chrono::system_clock::now();
	time_t curr_time = std::chrono::system_clock::to_time_t(curr_time_point);
	if (curr_time - newTransaction.creationTime < 0)
		return false;

	// check if the sender and the reciever is the same person
	if (newTransaction.from == newTransaction.to)
		return false;

	// run across the history of the blockchain to check:
	// - if the sender have the money for that transaction
	// - if no transaction with the same signiture was ever made
	Block* curr_block_ptr = m_firstBlock;
	Satoshi sender_balance = 0;
	Address sender = newTransaction.from;
	while (curr_block_ptr)
	{
		// if the sender is the miner of the block add 1000 to his balance
		if (curr_block_ptr->miner == sender)
		{
			sender_balance += MONEY_CREATED_FOR_THE_MINER_EACH_BLOCK;
		}

		std::vector<Transaction>::iterator trans_iter = curr_block_ptr->trans.begin();
		for (; trans_iter != curr_block_ptr->trans.end(); trans_iter++)
		{
			// if he received money - add the amount to his balance
			if (trans_iter->to == sender)
				sender_balance += trans_iter->amount;

			// if he was the miner of the block - add the transaction fee to his balance
			if (curr_block_ptr->miner == sender)
				sender_balance += trans_iter->fee;

			// if he was the sender - subtract it from his balance
			if (trans_iter->from == sender)
				sender_balance -= (trans_iter->amount + trans_iter->fee);

			// if the signiture is the same - the transaction was already made
			if (memcmp(trans_iter->signature, newTransaction.signature, SIG_LENGTH) == 0)
				return false;
		}

		curr_block_ptr = curr_block_ptr->next;
	}
	// check if the sender has enough money for the transaction entire cost
	if (sender_balance < (newTransaction.amount + newTransaction.fee))
		return false;

	return true;
}

int main()
{
	std::cout << "Hello Blockchain! Very nice to meet you! My name is main()" << std::endl;
	return 0;
}