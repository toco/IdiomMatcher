#!/usr/bin/env xcrun swift

//
//  Eval.swift
//  
//
//  Created by Tobias Conradi on 05.07.15.
//  Licensed under MIT License, see LICENSE for full text.
//

import Foundation


struct Match: Hashable, CustomStringConvertible {
	let name: String
	let startEA: UInt
	let endEA: UInt;
	init(name:String, startEA:UInt = 0, endEA:UInt = 0) {
		self.name = name
		self.startEA = startEA
		self.endEA = endEA
	}

	init(dict:[String:AnyObject]) {
		self.name = dict["patternName"] as? String ?? ""
		self.startEA = dict["startEA"] as? UInt ?? 0
		self.endEA = dict["endEA"] as? UInt ?? 0
	}

	var hashValue: Int {
		get {
			return Int(endEA);
		}
	}

	var description: String {
		return "\n\(_stdlib_getDemangledTypeName(self)): \(String(startEA, radix:16)) – \(String(endEA, radix:16)) \(name)"
	}

}
typealias Matches = Set<Match>

func == (lhs: Match, rhs: Match) -> Bool {
	return lhs.endEA == rhs.endEA
}

func evaluatedMatches(patternMatches:Matches, referenceData:Matches) -> (correctMatches:Matches, wrongMatches:Matches, foundReferenceMatches:Matches, notFoundReferenceMatches:Matches) {

	var correctMatches =  Matches()
	var wrongMatches = Matches()
	var foundReferenceMatches = Matches()
	var notFoundReferenceMatches = Matches()

	for patternMatch in patternMatches {
		for referenceMatch in referenceData {
			if patternMatch.endEA == referenceMatch.endEA {
				correctMatches.insert(patternMatch)
				foundReferenceMatches.insert(referenceMatch)
				break
			}
		}
	}
	wrongMatches = patternMatches.subtract(correctMatches)
	notFoundReferenceMatches = referenceData.subtract(foundReferenceMatches)
	return (correctMatches,wrongMatches,foundReferenceMatches,notFoundReferenceMatches);
}


func dictFromJSONPath(path:String) -> [String:AnyObject]? {
	var matchedDict = [String:AnyObject]()
	if let matchedData = NSData(contentsOfFile: path) {
		do {
			var dict: [String:AnyObject]?
			try dict = NSJSONSerialization.JSONObjectWithData(matchedData,options: NSJSONReadingOptions.MutableContainers) as? [String:AnyObject]
			if dict != nil	{
				matchedDict = dict!
			}

		} catch {
			print("Faild decoding \(path)")
		}
	}

	return matchedDict
}

func matchesFromDicts(dicts:[[String:AnyObject]]) -> Matches {
	var matches = Matches()
	for dict in dicts {
		matches.insert(Match(dict: dict))
	}
	return matches
}

func uniqueMatchesByRemovingLessFrequentPatterns(matches:Matches) -> Matches {

	let patternMatchNames = Set(matches.map { $0.name })
	var patternNameToNumber = [String:Int]()
	for name in patternMatchNames {
		patternNameToNumber[name] = matches.filter({$0.name == name}).count
	}
	var matchesToFilter = Set(matches)
	let matchEAs = Set(matches.map {$0.endEA})
	for ea in matchEAs {
		var matchesForEA = matches.filter({ $0.endEA == ea }).sort {patternNameToNumber[$0.name] < patternNameToNumber[$1.name] }
		matchesForEA.removeAtIndex(0)
		matchesToFilter.subtractInPlace(Set(matchesForEA))
	}
	return matchesToFilter
}

// MARK: -

let matchesPath = NSUserDefaults.standardUserDefaults().stringForKey("matched")
let referencePath = NSUserDefaults.standardUserDefaults().stringForKey("reference")

let verbose = NSUserDefaults.standardUserDefaults().boolForKey("verbose")

let csv = NSUserDefaults.standardUserDefaults().integerForKey("csv")

if matchesPath == nil || referencePath == nil {
	let name = NSProcessInfo.processInfo().processName;
	print("Usage:\n \(name) -matched PATH_TO_MATCHED.json -reference PATH_TO_REFERENCE.json [-verbose 0|1] [-csv 0|1|2] ")
	exit(1)
}

let matchesDictFull = dictFromJSONPath(matchesPath ?? "")
let matchesDict = matchesDictFull?["matches"] as? [[String:AnyObject]]
let realtime = matchesDictFull?["realTime"] as? Double ?? 0.0;
let cputime = matchesDictFull?["cpuTime"] as? Double ?? 0.0;
let architecture = matchesDictFull?["executableArchitecture"] as? String ?? ""

let executableName = matchesDictFull?["executableName"] as? String ?? ""
let matcherName = matchesDictFull?["matcherName"] as? String ?? ""

if (matchesDict == nil) {
	print("failed to read matched dict \(matchesPath)")
	exit(1)
}

let referenceDictFull = dictFromJSONPath(referencePath ?? "")
let referenceDict = referenceDictFull?["matches"] as? [[String:AnyObject]]

if (referenceDict == nil) {
	print("failed to read reference dict \(referencePath)")
	exit(1)
}

// MARK: -

let matches = matchesFromDicts(matchesDict!)
let references = matchesFromDicts(referenceDict!)

let filteredMatches = uniqueMatchesByRemovingLessFrequentPatterns(matches)

let (correctMatches, wrongMatches, foundReferenceMatches, notFoundReferenceMatches) = evaluatedMatches(filteredMatches, referenceData: references)

let patternsUsed = Set(correctMatches.map { $0.name })

let numberOfPatterns = patternsUsed.count

extension SequenceType where Generator.Element == Match {
	var sortByEA: [Generator.Element] {
		return self.sort {$0.startEA < $1.startEA}
	}
}

if csv > 0 {
	let correct = correctMatches.count
	let wrong = wrongMatches.count
	let notFound = notFoundReferenceMatches.count
	let sum = correct+notFound+wrong
/*
	var	correctp = 0.0
	var wrongp = 0.0
	var	notFoundp = 0.0
	if sum>0 {
		correctp = Double(correct)/Double(sum)
		wrongp = Double(wrong)/Double(sum)
		notFoundp = Double(notFound)/Double(sum)
	}
*/
	let f = NSNumberFormatter()
	f.minimumIntegerDigits = 1
	f.maximumFractionDigits = 5
	var sep :String
	if (csv == 1) {
	 sep = ", "
	 f.decimalSeparator = "."
	} else {
	 sep = "\t"
	 f.decimalSeparator = ","
	}

	if sum > 0 {
		let parts = [executableName,
			f.stringFromNumber(correct)!,
			f.stringFromNumber(notFound)!,
			f.stringFromNumber(wrong)!,
//			f.stringFromNumber(sum)!,
			f.stringFromNumber(realtime)!,
			f.stringFromNumber(cputime)!,
			matcherName,
			architecture,
			f.stringFromNumber(numberOfPatterns)!
		]
		print(parts.joinWithSeparator(sep))
	}
} else {
	print("correct: \(correctMatches.count)")
	if verbose{
		print(correctMatches.sortByEA)
	}
	print("wrong: \(wrongMatches.count)")

	print(wrongMatches.sortByEA)

	print("references not found: \(notFoundReferenceMatches.count)")
	print(notFoundReferenceMatches.sortByEA)

	print("number of patterns used: \(numberOfPatterns)")
}

