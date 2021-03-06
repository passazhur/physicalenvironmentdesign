#ifndef POLYNOMIAL
#define POLYNOMIAL

#include <iostream>
#include <list>

#define L1_id 1
#define L2_id 2
#define L3_id 3
#define L4_id 4

#define L1_literal "L1"
#define L2_literal "L2"
#define L3_literal "L3"
#define L4_literal "L3"

namespace FEM
{    
    class Polynomial
    {
        public : friend class Derivative;
        public : class Summand;

        class Variable
        {
            /*public : const std::string name;*/
            public : const int id = -1;
            public : Variable() noexcept : /*name("?"),*/ id(-1) {}
            public : Variable(/*const std::string &name,*/ int id) noexcept : /*name(name),*/ id(id){}
            public : Variable(const Variable &V) noexcept : /*name(V.name),*/ id(V.id){}
            public : Variable & operator = (const Variable &target) noexcept
            {
                //(*const_cast<std::string*>(&name)) = target.name;
                (*const_cast<int*>(&id)) = target.id;
                return *this;
            }
            public : bool operator == (const Variable &target) const noexcept
            {
                return id == target.id;
            }
            public : friend std::ostream & operator << (std::ostream &out, const Variable &V)
            {
                //return out << V.name;
                return out << "L" << V.id;
            }
            public : friend Polynomial::Summand operator * (
                    const int coef, const Variable &V) noexcept
            {
                return Polynomial::Summand(coef)*V;
            }

            public : ~Variable() noexcept {}
        };

        public : struct Multiplier // e.g. x^2
        {
            Variable var;
            int power;
        };
        public : class Summand // e.g. c*x^2*y
        {
            public : std::list<Multiplier> multipliers;
            public : int coef;

            public : Summand(const int coef) noexcept
            {
                this->coef = coef;
            }
            public : Summand(const Variable &var) noexcept
            {
                multipliers.push_back(Multiplier{var,1});
                coef = 1;
            }

            public : bool compare(const Summand &target) noexcept
            {
                if(multipliers.size() != target.multipliers.size())
                    return false;
                for(Multiplier _curMultiplier: multipliers)
                {
                    bool _isMissing = true;
                    for(Multiplier _curTargetMultiplier: target.multipliers)
                        if(_curMultiplier.var == _curTargetMultiplier.var &&
                                _curMultiplier.power == _curTargetMultiplier.power)
                        {
                            _isMissing = false;
                            break;
                        }
                    if(_isMissing) return false;
                }
                return true;
            }
            public : Summand & operator * (const int coef) noexcept
            {
                this->coef *= coef;
                return *this;
            }
            public : Summand & operator * (const Variable &var) noexcept
            {
                bool _isMissing = true;
                for(Multiplier &_curMultiplier: multipliers)
                    if(_curMultiplier.var == var)
                    {
                        _isMissing = false;
                        _curMultiplier.power += 1;
                        break;
                    }
                if(_isMissing) multipliers.push_back(Multiplier{var,1});
                return *this;
            }
        };

        public : std::list<Summand> summands;

        public : Polynomial() noexcept {}
        public : Polynomial(const Polynomial &target) noexcept : summands(target.summands){}
        public : Polynomial & operator = (const Polynomial &target) noexcept
        {
            summands.clear();
            summands = target.summands;
            return *this;
        }
        public : Polynomial(const int coeff) noexcept
        {
            Summand _s(coeff);
            summands.push_back(_s);
        }
        public : Polynomial(const Variable &V) noexcept
        {
            Summand _s(V);
            summands.push_back(_s);
        }
        public : Polynomial(const Summand &S) noexcept
        {
            summands.push_back(S);
        }

        public : friend Polynomial operator + (
                const Polynomial &A, const Polynomial &B) noexcept
        {
            Polynomial _rezult = A;
            for(auto _curBSummandIterator = B.summands.cbegin();
                _curBSummandIterator != B.summands.cend(); ++_curBSummandIterator)
            {
                bool _isMissing = true;
                for(auto _curSummandIterator = _rezult.summands.begin();
                    _curSummandIterator != _rezult.summands.end(); ++ _curSummandIterator)
                    if((*_curSummandIterator).compare(*_curBSummandIterator))
                    {
                        (*_curSummandIterator).coef += (*_curBSummandIterator).coef;
                        if((*_curSummandIterator).coef == 0)
                            _curSummandIterator = _rezult.summands.erase(_curSummandIterator);
                        _isMissing = false;
                        break;
                    }
                if(_isMissing)
                    _rezult.summands.push_back(*_curBSummandIterator);
            }
            return _rezult;
        }
        public : friend Polynomial operator - (
                const Polynomial &A, const Polynomial &B) noexcept
        {
            Polynomial _rezult = A;
            for(auto _curBSummandIterator = B.summands.cbegin();
                _curBSummandIterator != B.summands.cend(); ++_curBSummandIterator)
            {
                bool _isMissing = true;
                for(auto _curSummandIterator = _rezult.summands.begin();
                    _curSummandIterator != _rezult.summands.end(); ++ _curSummandIterator)
                    if((*_curSummandIterator).compare(*_curBSummandIterator))
                    {
                        (*_curSummandIterator).coef -= (*_curBSummandIterator).coef;
                        if((*_curSummandIterator).coef == 0)
                            _curSummandIterator = _rezult.summands.erase(_curSummandIterator);
                        _isMissing = false;
                        break;
                    }
                if(_isMissing)
                {
                    _rezult.summands.push_back(*_curBSummandIterator);
                    _rezult.summands.back().coef = - _rezult.summands.back().coef;
                }
            }
            return _rezult;
        }
        public : friend std::ostream & operator << (std::ostream &out, const Polynomial& P)
        {
            if(P.summands.empty())
            {
                out << "0";
                return out;
            }
            bool _isFirst = true;
            for(const Summand &_curSummand: P.summands)
            {
                if(_curSummand.coef > 0)
                {
                    if(!_isFirst)out << " + ";
                }
                else out << " - ";
                if(std::abs(_curSummand.coef) > 1 || _curSummand.multipliers.empty())
                {
                    out << std::abs(_curSummand.coef);
                    if(!_curSummand.multipliers.empty())
                        out << "*";
                }
                bool _isFirstMultiplier = true;
                for(const Multiplier &_curMultiplier: _curSummand.multipliers)
                {
                    if(!_isFirstMultiplier)out << "*";
                    out << _curMultiplier.var;
                    if(_curMultiplier.power > 1)
                        out << "^" << _curMultiplier.power;
                    _isFirstMultiplier = false;
                }
                _isFirst = false;
            }
            return out;
        }
        /// \todo more variables
        public : float calculate(
                const float vL1 = 0,
                const float vL2 = 0,
                const float vL3 = 0,
                const float vL4 = 0) const noexcept
        {
            float _result = 0.0f;
            for(const Summand &_curSummand: summands)
            {
                float _mul = _curSummand.coef;
                for(const Multiplier &_curMultiplier: _curSummand.multipliers)
                {
                    switch (_curMultiplier.var.id)
                    {
                        case 0: for(int i=0; i<_curMultiplier.power; ++i) _mul *= vL1; break;
                        case 1: for(int i=0; i<_curMultiplier.power; ++i) _mul *= vL2; break;
                        case 2: for(int i=0; i<_curMultiplier.power; ++i) _mul *= vL3; break;
                        case 3: for(int i=0; i<_curMultiplier.power; ++i) _mul *= vL4; break;
                    }
                }
                _result += _mul;
            }
            return _result;
        }
    };
}

#endif // POLYNOMIAL
