/**
 * @file lexicon.cpp
 */

#include "lexicon.h"

Lexicon::Lexicon(const string & lexiconFile):
    _lexiconFilename(lexiconFile)
{
    _entries = new unordered_map<TermID, TermData>();
    _docLengths = new unordered_map<DocID, unsigned int>();
    readLexicon();
    readDocLengths();
    setAvgDocLength();
}

Lexicon::~Lexicon()
{
    delete _entries;
    delete _docLengths;
}

Lexicon::Lexicon(const Lexicon & other)
{
    _entries = new unordered_map<TermID, TermData>(*other._entries);
    _docLengths = new unordered_map<DocID, unsigned int>(*other._docLengths);
    _lexiconFilename = other._lexiconFilename;
}

const Lexicon & Lexicon::operator=(const Lexicon & other)
{
    if(this != &other)
    {
        delete _entries;
        delete _docLengths;
        _entries = other._entries;
        _docLengths = other._docLengths;
        _lexiconFilename = other._lexiconFilename;
    }
    return *this;
}

TermData Lexicon::getTermInfo(TermID termID) const
{
    unordered_map<TermID, TermData>::const_iterator it = _entries->find(termID);
    if(it == _entries->end())
    {
        cerr << "[Lexicon]: warning: termID lookup failed" << endl;
        return TermData();
    }
    return it->second;
}

void Lexicon::save() const
{
    ofstream outfile(_lexiconFilename);
    if(outfile.good())
    {
        unordered_map<TermID, TermData>::const_iterator it;
        for(it = _entries->begin(); it != _entries->end(); ++it)
        {
            string line = toString(it->first) + " ";
            TermData data = it->second;
            line += toString(data.idf) + " ";
            line += toString(data.totalFreq) + " ";
            line += toString(data.postingIndex) + " ";
            line += toString(data.postingBit) + "\n";
            outfile << line;
        }
        outfile.close();
    }
    else
    {
        cerr << "[Lexicon]: error writing lexicon to disk" << endl;
    }
}

template <class T>
string Lexicon::toString(T value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

void Lexicon::addTerm(TermID term, TermData termData)
{
    _entries->insert(make_pair(term, termData));
}

void Lexicon::readLexicon()
{
    Parser parser(_lexiconFilename, "\n");
    if(!parser.isValid())
    {
        cerr << "[Lexicon]: created empty lexicon" << endl;
        return;
    }

    cerr << "[Lexicon]: reading from file..." << endl;

    // the first line in this file is the path to the doc lengths
    _lengthsFilename = parser.next();

    while(parser.hasNext())
    {
        istringstream line(parser.next());
        vector<string> items;
        copy(std::istream_iterator<string>(line),
             std::istream_iterator<string>(),
             std::back_inserter<vector<string>>(items));
        TermID termID;
        TermData data;
        istringstream(items[0]) >> termID;
        istringstream(items[1]) >> data.idf;
        istringstream(items[2]) >> data.totalFreq;
        istringstream(items[3]) >> data.postingIndex;
        istringstream(items[4]) >> data.postingBit;
        addTerm(termID, data);
    }
}

void Lexicon::readDocLengths()
{
    Parser parser(_lengthsFilename, " \n");

    while(parser.hasNext())
    {
        DocID docID;
        unsigned int length;
        istringstream(parser.next()) >> docID;
        istringstream(parser.next()) >> length;
        _docLengths->insert(make_pair(docID, length));
    }
}

unsigned int Lexicon::getDocLength(DocID docID) const
{
    return _docLengths->at(docID);
}

unsigned int Lexicon::getNumDocs() const
{
    return _docLengths->size();
}

double Lexicon::getAvgDocLength() const
{
    return _avgDL;
}

void Lexicon::setAvgDocLength()
{
    double sum = 0;
    for(auto it = _docLengths->begin(); it != _docLengths->end(); ++it)
        sum += it->second;

    _avgDL = (double) sum / _docLengths->size();
}
