//
// Created by Tobias Conradi on 02.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include <Model/Pattern.h>
#include <Model/DisassemblyPersistence.cpp>
#include <Model/PatternPersistence.cpp>

#include "rapidjson/stringbuffer.h"

#define BOOST_TEST_MODULE ModelTest
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(OperandInitalisation)
{
    IdiomMatcher::Operand operand("testOP");

    BOOST_CHECK(operand.getText() == "testOP");
    BOOST_CHECK_EQUAL(operand.getRegisters().size(),0);
    BOOST_CHECK(!operand.isWildcard());
    BOOST_CHECK(operand.getExtractAs() == "");
    BOOST_CHECK(operand.getRegex() == "");
    BOOST_CHECK(!operand.getNameIsTemplate());
    BOOST_CHECK(operand.getUsed());
    BOOST_CHECK(!operand.getModified());

    std::vector<std::string> registers;
    IdiomMatcher::Operand operand2("",registers,"extractName");
    BOOST_CHECK(operand2.getText() == "");
    BOOST_CHECK_EQUAL(operand2.getRegisters().size(),0);
    BOOST_CHECK(operand2.isWildcard());
    BOOST_CHECK(operand2.getExtractAs() == "extractName");
    BOOST_CHECK(operand2.getRegex() == "");
    BOOST_CHECK(!operand2.getNameIsTemplate());
    BOOST_CHECK(operand2.getUsed());
    BOOST_CHECK(!operand2.getModified());
	BOOST_CHECK_EQUAL(operand2.getAddress(),0);

    registers.push_back("reg");
    IdiomMatcher::Operand operand3("A",registers,"extractName","regexString",true,false,true,1);
    BOOST_CHECK(operand3.getText() == "A");
    BOOST_CHECK_EQUAL(operand3.getRegisters().size(),1);
    BOOST_CHECK(operand3.getRegisters().front() == "reg");
    BOOST_CHECK(!operand3.isWildcard());
    BOOST_CHECK(operand3.getExtractAs() == "extractName");
    BOOST_CHECK(operand3.getRegex() == "regexString");
    BOOST_CHECK(operand3.getNameIsTemplate());
    BOOST_CHECK(!operand3.getUsed());
    BOOST_CHECK(operand3.getModified());
	BOOST_CHECK_EQUAL(operand3.getAddress(),1);


    registers.push_back("regOther");
    IdiomMatcher::Operand operand4("op",registers,false, true,2);
    BOOST_CHECK_EQUAL(operand4.getText(),"op");
    BOOST_CHECK_EQUAL(operand4.getRegisters().size(),2);
    BOOST_CHECK(operand4.getRegisters().front() == "reg");
    BOOST_CHECK(operand4.getRegisters().back() == "regOther");
    BOOST_CHECK_EQUAL(operand4.isWildcard(), false);
    BOOST_CHECK_EQUAL(operand4.getExtractAs(), "");
    BOOST_CHECK_EQUAL(operand4.getRegex(), "");
    BOOST_CHECK_EQUAL(operand4.getNameIsTemplate(),false);
    BOOST_CHECK_EQUAL(operand4.getUsed(), false);
    BOOST_CHECK_EQUAL(operand4.getModified(), true);
	BOOST_CHECK_EQUAL(operand4.getAddress(),2);
}

using namespace rapidjson;
typedef GenericDocument<ASCII<>, MemoryPoolAllocator<>, MemoryPoolAllocator<>> DocumentType;

BOOST_AUTO_TEST_CASE(OperandSerialization)
{
    std::vector<std::string> registers;
    registers.push_back("reg1");
    IdiomMatcher::Operand operand("A",registers,"extractName","regexString",true,false,true);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    SerializeOperand(writer,operand);
    DocumentType d;
    d.Parse(buffer.GetString());

    IdiomMatcher::Operand operand1 = *IdiomMatcher::operandFromJSON(d);;
    BOOST_CHECK(operand1.getText() == "A");
    BOOST_CHECK_EQUAL(operand1.getRegisters().size(), 1);
    BOOST_CHECK(operand1.getRegisters().front() == "reg1");
    BOOST_CHECK(!operand1.isWildcard());
    BOOST_CHECK(operand1.getExtractAs() == "extractName");
    BOOST_CHECK(operand1.getRegex() == "regexString");
    BOOST_CHECK(operand1.getNameIsTemplate());
    BOOST_CHECK(!operand1.getUsed());
    BOOST_CHECK(operand1.getModified());

    IdiomMatcher::Operand operand3("B");
    buffer.Clear();
	writer.Reset(buffer);
    SerializeOperand(writer,operand3);
    d.RemoveAllMembers();
    d.Parse(buffer.GetString());

    IdiomMatcher::Operand operand4 = *IdiomMatcher::operandFromJSON(d);;
    BOOST_CHECK_EQUAL(operand4.getText(), "B");
    BOOST_CHECK_EQUAL(operand4.getRegisters().size(), 0);
    BOOST_CHECK_EQUAL(operand4.isWildcard(),false);
    BOOST_CHECK_EQUAL(operand4.getExtractAs(), "");
    BOOST_CHECK_EQUAL(operand4.getRegex(), "");
    BOOST_CHECK_EQUAL(operand4.getNameIsTemplate(), false);
    BOOST_CHECK_EQUAL(operand4.getUsed(), true);
    BOOST_CHECK_EQUAL(operand4.getModified(), false);
}

BOOST_AUTO_TEST_CASE(XRefInitalization)
{
	{
		IdiomMatcher::XRef xref(10);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 10);
		BOOST_CHECK_EQUAL(xref.isData(), false);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), false);
	}
	{
		IdiomMatcher::XRef xref(10,true);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 10);
		BOOST_CHECK_EQUAL(xref.isData(), true);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), false);
	}
	{
		IdiomMatcher::XRef xref(10,true,true);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 10);
		BOOST_CHECK_EQUAL(xref.isData(), true);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), true);
	}
	{
		IdiomMatcher::XRef xref(10,false,true);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 10);
		BOOST_CHECK_EQUAL(xref.isData(), false);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), true);
	}
	{
		IdiomMatcher::XRef xref(10,true,false);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 10);
		BOOST_CHECK_EQUAL(xref.isData(), true);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), false);
	}
}

BOOST_AUTO_TEST_CASE(XrefSerialization)
{


	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	{
		IdiomMatcher::XRef xref(IdiomMatcher::EA(2));
		SerializeXRef(writer,xref);
	}
	DocumentType d;
	d.Parse(buffer.GetString());
	{
		IdiomMatcher::XRef xref = *IdiomMatcher::xrefFromJSON(d);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 2);
		BOOST_CHECK_EQUAL(xref.isData(), false);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), false);
	}

	buffer.Clear();
	writer.Reset(buffer);
	d.RemoveAllMembers();

	{
		IdiomMatcher::XRef xref(IdiomMatcher::EA(10),true,false);
		SerializeXRef(writer,xref);
	}

	d.Parse(buffer.GetString());
	{
		IdiomMatcher::XRef xref = *IdiomMatcher::xrefFromJSON(d);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 10);
		BOOST_CHECK_EQUAL(xref.isData(), true);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), false);
	}

	buffer.Clear();
	writer.Reset(buffer);
	d.RemoveAllMembers();

	{
		IdiomMatcher::XRef xref(IdiomMatcher::EA(5),true,true);
		SerializeXRef(writer,xref);
	}

	d.Parse(buffer.GetString());
	{
		IdiomMatcher::XRef xref = *IdiomMatcher::xrefFromJSON(d);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 5);
		BOOST_CHECK_EQUAL(xref.isData(), true);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), true);
	}

	d.RemoveMember("isUnordinaryFlow");
	d.RemoveMember("isData");

	{
		IdiomMatcher::XRef xref = *IdiomMatcher::xrefFromJSON(d);
		BOOST_CHECK_EQUAL(xref.getTarget().getValue(), 5);
		BOOST_CHECK_EQUAL(xref.isData(), false);
		BOOST_CHECK_EQUAL(xref.isUnordinaryFlow(), false);
	}

}


BOOST_AUTO_TEST_CASE(InstructionInitalization)
{
    IdiomMatcher::Operands operands;
    operands.push_back(std::make_shared<IdiomMatcher::Operand>("testOP"));
	IdiomMatcher::XRefs xrefs;

	IdiomMatcher::Instruction instruction("testInstruc",operands,xrefs);
    BOOST_CHECK(instruction.getMnemonic() == "testInstruc");
    BOOST_CHECK(instruction.getOperands() == operands);
	BOOST_CHECK(instruction.getXrefs() == xrefs);
    BOOST_CHECK_EQUAL(instruction.getSize(), 0);
	BOOST_CHECK(instruction.getEA()==IdiomMatcher::InvalidEA);
	BOOST_CHECK(!instruction.getIsRegex());

    IdiomMatcher::Instruction instruction1("testInstruc1",operands,xrefs,2,IdiomMatcher::EA(3),true);
    BOOST_CHECK(instruction1.getMnemonic() == "testInstruc1");
    BOOST_CHECK(instruction1.getOperands() == operands);
	BOOST_CHECK(instruction1.getXrefs() == xrefs);
	BOOST_CHECK_EQUAL(instruction1.getSize(), 2);
	BOOST_CHECK_EQUAL(instruction1.getEA().getValue(), 3);
	BOOST_CHECK(instruction1.getIsRegex());
}

BOOST_AUTO_TEST_CASE(InstructionSerialization)
{

    IdiomMatcher::Operands operands;
    operands.push_back(std::make_shared<IdiomMatcher::Operand>("testOP"));
    operands.push_back(std::make_shared<IdiomMatcher::Operand>("testOP1"));
	IdiomMatcher::XRefs xrefs;
	xrefs.push_back(std::make_shared<IdiomMatcher::XRef>(IdiomMatcher::EA(10),true));
	xrefs.push_back(std::make_shared<IdiomMatcher::XRef>(IdiomMatcher::EA(2)));

	IdiomMatcher::Instruction instruction("testInstruc",operands,xrefs,2,IdiomMatcher::EA(3),true);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    SerializeInstruction(writer,instruction);
    DocumentType d;
    d.Parse(buffer.GetString());

    instruction = *IdiomMatcher::instructionFromJSON(d);

    BOOST_CHECK(instruction.getMnemonic() == "testInstruc");

    BOOST_CHECK(instruction.getOperands().front()->getText() == "testOP");
    BOOST_CHECK(instruction.getOperands().back()->getText() == "testOP1");
    BOOST_CHECK_EQUAL(instruction.getOperands().size(), 2);

	BOOST_CHECK(instruction.getXrefs().front()->getTarget() == IdiomMatcher::EA(10));
	BOOST_CHECK(instruction.getXrefs().front()->isData());
	BOOST_CHECK(instruction.getXrefs().back()->getTarget() == IdiomMatcher::EA(2));
	BOOST_CHECK(!instruction.getXrefs().back()->isData());
	BOOST_CHECK_EQUAL(instruction.getXrefs().size(), 2);

    BOOST_CHECK_EQUAL(instruction.getSize(), 2);
	BOOST_CHECK_EQUAL(instruction.getEA().getValue(), 3);
	BOOST_CHECK(instruction.getIsRegex());
}

BOOST_AUTO_TEST_CASE(ActionInitialization) {
    IdiomMatcher::Action action("testScript");
    BOOST_CHECK(action.getScript() == "testScript");
}

BOOST_AUTO_TEST_CASE(ActionSerialization)
{
    IdiomMatcher::Action action("testScript");

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    IdiomMatcher::SerializeAction(writer,action);
    DocumentType d;
    d.Parse(buffer.GetString());

    IdiomMatcher::Action action1 = *IdiomMatcher::actionFromJSON(d);
    BOOST_CHECK(action1.getScript() == "testScript");
}

BOOST_AUTO_TEST_CASE(PatternInitialisation)
{

    std::vector<std::string> registers;
    registers.push_back("reg1");
    IdiomMatcher::Operands operands;
    operands.push_back(std::make_shared<IdiomMatcher::Operand>("A",registers,"extractName","",true,false,true));
    operands.push_back(std::make_shared<IdiomMatcher::Operand>("testOP"));

	IdiomMatcher::XRefs refs;

    IdiomMatcher::Instructions instructions;
    instructions.push_back(std::make_shared<IdiomMatcher::Instruction>("testInstruc1",operands,refs,2));
    instructions.push_back(std::make_shared<IdiomMatcher::Instruction>("testInstruc2",operands,refs,3));

    IdiomMatcher::Actions actions;
    IdiomMatcher::Pattern pattern("testPattern",instructions,"myArch", actions, true);

    BOOST_CHECK(pattern.getName() == "testPattern");
    BOOST_CHECK(pattern.getInstructions() == instructions);
    BOOST_CHECK(pattern.getActions() == actions);
    BOOST_CHECK(pattern.getAnchorTop() == true);
	BOOST_CHECK(pattern.getArchitecture() == "myArch");
}

BOOST_AUTO_TEST_CASE(PatternSerialization)
{

    std::vector<std::string> registers;
    auto operand1 = std::make_shared<IdiomMatcher::Operand>("A",registers,"extractName","",true,false,true);
    auto operand2 = std::make_shared<IdiomMatcher::Operand>("testOP");

    IdiomMatcher::Operands operands1;
    operands1.push_back(operand1);
    operands1.push_back(operand2);

    IdiomMatcher::Operands operands2;
    operands2.push_back(operand2);

	IdiomMatcher::XRefs refs;

    IdiomMatcher::Instructions instructions1;
    instructions1.push_back(std::make_shared<IdiomMatcher::Instruction>("testInstruc1", operands1,refs,2));
    instructions1.push_back(std::make_shared<IdiomMatcher::Instruction>("testInstruc2", operands2,refs,3));

    IdiomMatcher::Actions actions;
    IdiomMatcher::Pattern pattern("testPattern", instructions1, "someArch", actions, true);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    IdiomMatcher::SerializePattern(writer,pattern);
    DocumentType d;
    d.Parse(buffer.GetString());

    IdiomMatcher::Pattern pattern2 = *IdiomMatcher::patternFromJSON(d);

    BOOST_CHECK(pattern2.getName() == "testPattern");
    BOOST_CHECK_EQUAL(pattern2.getActions().size(), 0);
    BOOST_CHECK_EQUAL(pattern2.getAnchorTop(), true);
	BOOST_CHECK_EQUAL(pattern2.getArchitecture(), "someArch");


    auto instructions2 = pattern2.getInstructions();
    BOOST_CHECK_EQUAL(instructions2.size(),2);

    auto instruction1 = instructions2.front();
    BOOST_CHECK(instruction1->getMnemonic() == "testInstruc1");
    BOOST_CHECK_EQUAL(instruction1->getSize(), 2);

    auto operands3 = instruction1->getOperands();
    BOOST_CHECK_EQUAL(operands3.size(), 2);
    auto operand3 = operands3.front();
    BOOST_CHECK(operand3->getText() == "A");
    BOOST_CHECK(!operand3->isWildcard());
    BOOST_CHECK(operand3->getExtractAs() == "extractName");
    BOOST_CHECK(operand3->getNameIsTemplate());
    BOOST_CHECK(!operand3->getUsed());
    BOOST_CHECK(operand3->getModified());

    auto operand4 = operands3.back();
    BOOST_CHECK(operand4->getText() == "testOP");

    auto instruction2 = instructions2.back();
    BOOST_CHECK(instruction2->getMnemonic() == "testInstruc2");
    BOOST_CHECK_EQUAL(instruction2->getSize(), 3);

    BOOST_CHECK_EQUAL(instruction2->getOperands().size(),1);
    BOOST_CHECK(instruction2->getOperands().front()->getText() == "testOP");
}
