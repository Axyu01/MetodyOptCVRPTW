#ifndef SOLUTION_H
#define SOLUTION_H


class Solution
{
    public:
        Solution(Solution* s);
        Solution(int size);
        Solution(Solution*s,int size);
        virtual ~Solution();
        int* Genome;
        int size;
        double eval;
        double penalty;
        print();
        Insert(int g,int position);

    protected:

    private:
};

#endif // SOLUTION_H
