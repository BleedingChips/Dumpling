#include "parser_lr0.h"
#include <optional>
namespace Potato::Parser::Lr0
{

	std::any History::operator()(std::any(*Function)(void*, Element&), void* FunctionBody) const
	{
		std::vector<std::tuple<Symbol, std::any>> DataBuffer;
		for (auto& ite : steps)
		{
			Element Re(ite);
			if (ite.value.IsTerminal())
			{
				auto Result = (*Function)(FunctionBody, Re);
				DataBuffer.push_back({ ite.value,  std::move(Result) });
			}
			else {
				assert(DataBuffer.size() >= ite.reduce.production_count);
				size_t CurrentAdress = DataBuffer.size() - ite.reduce.production_count;
				Re.datas = DataBuffer.data() + CurrentAdress;
				auto Result = (*Function)(FunctionBody, Re);
				DataBuffer.resize(CurrentAdress);
				DataBuffer.push_back({ ite.value, std::move(Result) });
			}
		}
		assert(DataBuffer.size() == 1);
		return std::move(std::get<1>(DataBuffer[0]));
	}

	struct SearchElement
	{
		size_t StepsRecord;
		std::vector<size_t> State;
		size_t TokenIndex;
	};

	bool HandleInputToken(Table const& Table, std::vector<Step>& Steps, SearchElement& SE, Symbol Sym, size_t token_index)
	{
		if (!Steps.empty() && Sym.IsTerminal())
		{
			auto LastStep = *Steps.rbegin();
			if (LastStep.value.IsNoTerminal())
			{
				auto& RefProduction = Table.productions[LastStep.reduce.production_index].cancle_reduce;
				if (RefProduction.find(Sym) != RefProduction.end())
					return false;
			}
		}

		auto CurState = *SE.State.rbegin();
		auto Nodes = Table.nodes[CurState];
		for (size_t i = 0; i < Nodes.shift_count; ++i)
		{
			auto TarShift = Table.Shifts[i + Nodes.shift_adress];
			if (TarShift.RequireSymbol == Sym)
			{
				SE.State.push_back(TarShift.ShiftState);
				if (Sym.IsTerminal() && !Sym.IsEndOfFile())
				{
					Step ShiftStep;
					ShiftStep.value = Sym;
					ShiftStep.shift.token_index = token_index;
					Steps.push_back(ShiftStep);
					++SE.StepsRecord;
				}
				return true;
			}
		}
		return false;
	}

	History Process(Table const& Table, Symbol const* TokenArray, size_t TokenLength)
	{
		std::vector<Step> Steps;
		size_t MaxTokenUsed = 0;
		std::vector<Step> BackupSteps;
		std::vector<std::tuple<SearchElement, size_t>> SearchStack;;
		SearchElement CurSearchIte = SearchElement{ 0, {0}, 0 };
		do
		{	
			auto CurNode = Table.nodes[*CurSearchIte.State.rbegin()];
			for (size_t i = 0; i < CurNode.reduce_count; ++i)
				SearchStack.push_back({ CurSearchIte, CurNode.reduce_count - 1 - i });
			Symbol Sym;
			if (CurSearchIte.TokenIndex < TokenLength)
				Sym = TokenArray[CurSearchIte.TokenIndex];
			else
				Sym = Symbol::EndOfFile();
			bool Re = HandleInputToken(Table, Steps, CurSearchIte, Sym, CurSearchIte.TokenIndex++);
			if (!Re)
			{
				if (MaxTokenUsed < CurSearchIte.TokenIndex)
				{
					MaxTokenUsed = CurSearchIte.TokenIndex;
					BackupSteps = Steps;
				}
				if (!SearchStack.empty())
				{
					auto [Element, Index] = std::move(*SearchStack.rbegin());
					SearchStack.pop_back();
					auto OldStete = *Element.State.rbegin();
					auto CurNode = Table.nodes[*Element.State.rbegin()];
					auto Reduce = Table.Reduces[CurNode.reduce_adress + Index];
					auto Production = Table.productions[Reduce.ProductionIndex];
					Steps.resize(Element.StepsRecord);
					Step ReduceStep;
					ReduceStep.value = Production.value;
					ReduceStep.reduce.production_index = Reduce.ProductionIndex;
					ReduceStep.reduce.production_count = Production.production_count;
					ReduceStep.reduce.mask = Production.mask;
					Steps.push_back(ReduceStep);
					Element.StepsRecord += 1;
					Element.State.resize(Element.State.size() - Production.production_count);
					bool Re = HandleInputToken(Table, Steps, Element, Production.value, 0);
					assert(Re);
					CurSearchIte = std::move(Element);
					continue;
				}
				else {
					throw Error::unacceptable_symbol{ MaxTokenUsed, MaxTokenUsed < TokenLength ? TokenArray[MaxTokenUsed] : Symbol::EndOfFile(), std::move(BackupSteps) };
				}
			}
			else {
				if (Sym == Symbol::EndOfFile())
					return { std::move(Steps) };
			}
		} while (true);
	}

	std::set<Symbol> CalNullableSet(const std::vector<ProductionInput>& production)
	{
		std::set<Symbol> result;
		bool set_change = true;
		while (set_change)
		{
			set_change = false;
			for (auto& ite : production)
			{
				assert(ite.production.size() >= 1);
				if (ite.production.size() == 1)
				{
					set_change |= result.insert(ite.production[0]).second;
				}
				else {
					bool nullable_set = true;
					for (size_t index = 1; index < ite.production.size(); ++index)
					{
						Symbol symbol = ite.production[index];
						if (symbol.IsTerminal() || result.find(symbol) == result.end())
						{
							nullable_set = false;
							break;
						}
					}
					if (nullable_set)
						set_change |= result.insert(ite.production[0]).second;
				}
			}
		}
		return result;
	}

	std::map<Symbol, std::set<Symbol>> CalNoterminalFirstSet(
		const std::vector<ProductionInput>& production
	)
	{
		auto nullable_set = CalNullableSet(production);
		std::map<Symbol, std::set<Symbol>> result;
		bool set_change = true;
		while (set_change)
		{
			set_change = false;
			for (auto& ite : production)
			{
				assert(ite.production.size() >= 1);
				for (size_t index = 1; index < ite.production.size(); ++index)
				{
					auto head = ite.production[0];
					auto target = ite.production[index];
					if (target.IsTerminal())
					{
						set_change |= result[head].insert(target).second;
						break;
					}
					else {
						if (nullable_set.find(target) == nullable_set.end())
						{
							auto& ref = result[head];
							auto find = result.find(target);
							if (find != result.end())
								for (auto& ite3 : find->second)
									set_change |= ref.insert(ite3).second;
							break;
						}
					}
				}
			}
		}
		return result;
	}

	struct ProductionIndex
	{
		size_t index;
		size_t element_index;
		bool operator<(const ProductionIndex& pe) const
		{
			return index < pe.index || (index == pe.index && element_index < pe.element_index);
		}
		bool operator==(const ProductionIndex& pe) const
		{
			return index == pe.index && element_index == pe.element_index;
		}
	};

	std::set<ProductionIndex> ExpandProductionIndexSet(std::set<ProductionIndex> const& Pro, std::vector<ProductionInput> const& Productions, std::vector<Symbol> const& AddProduction)
	{
		std::set<ProductionIndex> Result;
		std::vector<ProductionIndex> SearchStack(Pro.begin(), Pro.end());
		while (!SearchStack.empty())
		{
			auto Ptr = *SearchStack.rbegin();
			SearchStack.pop_back();
			auto Re = Result.insert(Ptr);
			if (Re.second)
			{
				std::vector<Symbol> const* Target;
				if (Ptr.index < Productions.size())
					Target = &(Productions[Ptr.index].production);
				else
					Target = &AddProduction;
				size_t SymbolIndex = Ptr.element_index;
				if (SymbolIndex< Target->size())
				{
					auto TargetSymbol = (*Target)[SymbolIndex];
					if (TargetSymbol.IsNoTerminal())
					{
						for (size_t i = 0; i < Productions.size(); ++i)
						{
							if (Productions[i].production[0] == TargetSymbol)
								SearchStack.push_back({ i, 1 });
						}
						if (AddProduction[0] == TargetSymbol)
							SearchStack.push_back({ Productions.size(), 1 });
					}
				}
			}
		}
		return Result;
	}

	std::map<Symbol, std::set<Symbol>> TranslateOperatorPriority(
		const std::vector<OpePriority>& priority
	)
	{
		std::map<Symbol, std::set<Symbol>> ope_priority;
		size_t index = 0;
		for (size_t index = 0; index < priority.size(); ++index)
		{
			std::set<Symbol> current_remove;
			size_t target = index;
			auto& [opes, left] = priority[index];
			if (left == Associativity::Right)
				++target;
			for (; target > 0; --target)
			{
				auto& [opes, left] = priority[target - 1];
				for (auto& ite : opes)
					current_remove.insert(ite);
			}
			for (auto& ite : opes)
			{
				auto re = ope_priority.insert({ ite, current_remove });
				if (!re.second)
					throw Error::operator_priority_conflict{ ite, re.first->first };
			}
		}
		return std::move(ope_priority);
	}

	Table CreateTable(Symbol start_symbol, std::vector<ProductionInput> const& production, std::vector<OpePriority> const& priority)
	{
		Table result;

		auto OperatorPriority = TranslateOperatorPriority(priority);

		for (auto& ite : production) {
			Table::Production P{ ite.production[0], ite.production.size() - 1 , ite.function_mask, ite.remove };
			if (ite.production.size() >= 3)
			{
				auto Sym = ite.production[ite.production.size() - 2];
				if (Sym.IsTerminal())
				{
					auto FindIte = OperatorPriority.find({ Sym });
					if (FindIte != OperatorPriority.end())
						P.cancle_reduce = FindIte->second;
				}
			}
			result.productions.push_back(std::move(P));
		}
			

		std::vector<Symbol> AppendProduction = {Symbol::StartSymbol(), start_symbol, Symbol::EndOfFile()};

		std::set<ProductionIndex> PP({ ProductionIndex{ production.size(), 1} });
		PP = ExpandProductionIndexSet(PP, production, AppendProduction);
		std::map<std::set<ProductionIndex>, size_t> StateMapping;
		auto First = StateMapping.insert({ PP, 0 });
		std::vector<decltype(StateMapping)::const_iterator> StateMappingVec({ First.first });
		for (size_t i = 0; i < StateMapping.size(); ++i)
		{
			auto Cur = StateMappingVec[i];
			std::map<Symbol, std::set<ProductionIndex>> ShiftMapping;
			std::set<size_t> ReduceState;
			for (auto& Ite : Cur->first)
			{
				std::vector<Symbol> const* Target;
				if (Ite.index < production.size())
					Target = &(production[Ite.index].production);
				else
					Target = &AppendProduction;
				if (Ite.element_index < Target->size())
				{
					auto Re = ShiftMapping.insert({ (*Target)[Ite.element_index], {{Ite.index, Ite.element_index + 1}} });
					if (!Re.second)
						Re.first->second.insert({ Ite.index, Ite.element_index + 1 });
				}
				else {
					ReduceState.insert(Ite.index);
				}
			}
			std::vector<Table::Shift> Shiftting;
			for (auto& ite : ShiftMapping)
			{
				auto ExpanedSet = ExpandProductionIndexSet(ite.second, production, AppendProduction);
				auto Inserted = StateMapping.insert({ std::move(ExpanedSet), StateMapping.size() });
				if(Inserted.second)
					StateMappingVec.push_back(Inserted.first);
				Shiftting.push_back({ ite.first, Inserted.first->second });
			}
			std::vector<Table::Reduce> Reduceing;
			for (auto& ite : ReduceState)
				Reduceing.push_back(Table::Reduce{ ite });
			result.nodes.push_back({result.Reduces.size(), Reduceing.size(), result.Shifts.size(),  Shiftting.size()});
			result.Reduces.insert(result.Reduces.end(), Reduceing.begin(), Reduceing.end());
			result.Shifts.insert(result.Shifts.end(), Shiftting.begin(), Shiftting.end());
		}
		return result;
	}
}