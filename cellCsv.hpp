/*
Copyright (c) 2015, Daniel Fialkovsky
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of CellCSV nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _ccsv_cellCSV_HPP_
#define _ccsv_cellCSV_HPP_ 1

#include <unordered_map>
#include <vector>
#include <deque>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <fstream>

namespace ccsv 
{
    typedef unsigned long long numType;
    const numType COLUMN_LIMIT = 16777216; //(2^24) That leaves us with 2^40 rows 
    const std::string EMPTY_STRING = ""; // for reference passing errors with empty strings
    enum class dattype : int
    {
	INT,
	REAL,
	STRING
    };
    struct key
    {
	numType x;
	numType y;
	key(numType _x, numType _y): x(_x), y(_y) {}

    };
    struct keyCompare
    {
	bool operator()(const key &lhs, const key &rhs) const
	{
	    return lhs.x+(COLUMN_LIMIT*lhs.y) < rhs.x+(COLUMN_LIMIT*rhs.y);	    
	}
	
    };
    struct keyEqual
    {
	bool operator()(const key &lhs, const key &rhs) const
	{ 
	    return (lhs.x == rhs.x && lhs.y == rhs.y);
	}
    };
    
    struct keyHasher
    {
	std::size_t operator()(const key& k) const
	{
	    return k.x ^ (k.y << 1) ;
	}
};
    
    
    struct value
    {
    std::string 	str;
	dattype 	dtype;
    value(std::string _str, dattype _dtype) :str(_str), dtype(_dtype) {}
    };
    
    typedef std::pair<key,value> mapEntry;
    

    
    
    /*!
     * This is a class/library csv reader/writer.
     * Unlike many other csv writers, cellCsv is capable of 
     * adding contents to any "cell" in one go regardless if your data 
     * is being processed in different loops/times
     * @example usage you can call
     * 		csvHandle handler;
     * 		handler.setCell(3,5,"im bob");
     * 		handler.setCell(6,6,6.7345);
     * 		handler.setCell(0,3,3);
     * 		handler.dump("testcsv.csv");
     * 
     * It should be noted that this convience comes with a penalty of 
     * 	a) Using a hash table implementation, so this library is not completely memory efficient
     *  b) Using a vector to store keys so that the bounds can be kept track of
     * 
     * currently cellCsv doesn't natively handle producing fully rectangular csv files if the entries are not complete
     * 		 however this may be supported in the future
     */
    class cellCsv 
    {
    public:
	cellCsv(numType hashtableSize = 1500)
	{
	    boundVec_.reserve(hashtableSize);
	    csvData_.reserve(hashtableSize);
	}
	virtual ~cellCsv()
	{}
	static bool compareKeys(const key &lhs, const key& rhs)
	{
	    return lhs.x+(COLUMN_LIMIT*lhs.y) < rhs.x+(COLUMN_LIMIT*rhs.y);
	}
    public: 
	/*! inserts a string type into the specified cell
	 * @param x the x coordnate of the cell (starting from 0)
	 * @param y the y coordinate of the cell(starting from 0)
	 * @param str the string to be inserted
	 */
    void setCell(numType x, numType y, std::string str)
	{
	    if (str.empty()){
		return;
	    }
	    key k(x,y);
        value val(str, dattype::STRING);
	    insertToCell(k ,val);
	}
	void setCellReal(numType x, numType y, double dnum)
	{
	    key k(x,y);
        value val(std::to_string(dnum), dattype::REAL);
        insertToCell(k ,val);

	}
	
	void setCellInt(numType x, numType y, long long int num)
	{
	    key k(x,y);
        value val(std::to_string(num), dattype::INT);
	    insertToCell(k ,val);
	}
	
	/*! returns a copy of ccsv::value the coordinates given (access string data with retval.str)
	 * If for whatever reason you needed to keep track of the datatype, it can be retrieved with retval.dtype
	 * @param x the x coordnate of the cell (starting from 0)
	 * @param y the y coordinate of the cell(starting from 0)
	 * @return the datavalue that was stored in the cell 
	 * 
    */
	value getVal(numType x, numType y)
	{
	    auto It = csvData_.find(key(x,y));
	    if (It != csvData_.end()) // if false, then key not found!
	    {
		return It->second;
	    }
	    return value(EMPTY_STRING, dattype::STRING);
    }
	
	/*! returns a copy of the string the contents at the coordinates given
	 * If for whatever reason you needed to keep track of the datatype, it can be retrieved with retval.dtype
	 * @param x the x coordnate of the cell (starting from 0)
	 * @param y the y coordinate of the cell(starting from 0)
	 * @return the datavalue that was stored in the cell 
	 * 
    */
    std::string at(numType x, numType y)
	{
	    const auto cIt = csvData_.find(key(x,y));
	    if (cIt != csvData_.end()) // if false, then key not found!
	    {
		return cIt->second.str;
	    }
	    return EMPTY_STRING;
    }
	/*! returns a reference of the string the contents at the coordinates given
	 * If for whatever reason you needed to keep track of the datatype, it can be retrieved with retval.dtype
	 * @param x the x coordnate of the cell (starting from 0)
	 * @param y the y coordinate of the cell(starting from 0)
	 * @return the datavalue that was stored in the cell 
	 * 
    */
    const std::string & atCRef(numType x, numType y)
	{
	    const auto cIt = csvData_.find(key(x,y));
	    if (cIt != csvData_.end()) // if false, then key not found!
	    {
		return csvData_.at(key(x,y)).str;
	    }
	    return EMPTY_STRING;
    }
	/*! writes the csv file
	 * @param file the destination file/path for the csv file (know that you have to make the .csv extention on your own)
	 * @param delimiters optional parameter to add your own delimiters (if not using comma seperation)
	 * 
	*/
    void dump(std::string file, std::string delimiters = ",")
    {
        std::ofstream writeFile;
        writeFile.open(file.c_str(), std::ios::in | std::ios::trunc);
	    if (!writeFile.is_open()) // some file permissions error probably :(
		return;
	    numType colIter = 0;
	    numType rowIter = 0;
        std::string buf;
	    for (const key & akey : boundVec_) // boundVec_ is sorted so all 
	    {
		if (rowIter < akey.y)
		{
		    while (rowIter < akey.y)
		    {
            buf.push_back('\n');
			++rowIter;
		    }
            writeFile.write(buf.c_str(), buf.size());
            buf.clear();
		    colIter = 0;
		}
		while( colIter < akey.x)
		{
            buf += delimiters;
		    ++colIter;
		}	       
		
        buf += csvData_.at(akey).str;

	   } // end for
       if (!buf.empty())
	   {
// 		buf.push_back('\n');  // not sure if this is needed at all. 
        writeFile.write(buf.c_str(), buf.size());
        buf.clear();
	   }
	    
    }
	/*! Removes all the items from every cell. It however isn't garantueed to clear the memory that consumed (as of this implementation)
	 */
	void reset()
	{
	   csvData_.clear();
	   boundVec_.clear();
	}
	/*! @warning this hasn't been implemented yet
	 * removes the element from the key supplied
	 * @param x the x coordnate of the cell (starting from 0)
	 * @param y the y coordinate of the cell(starting from 0)
     */
    /*
	void remove(numType x, numType y)
	{
	    // to be implemented
    }
    */
	
    private:
	/*! adds a reference copy (in the future this will be a pointer or reference) to a boundVec_ to easily access data stored in the csvData_ hashtable
	 * @param k the key that corresponds to what should have been recently inserted into the hashtable 
	 */
	void addToBoundVec(const key & k)
	{
	    boundVec_.push_back(k);
// 	    gfx::TimSort<key,keyCompare>(boundVec_.begin(),boundVec_.end(), compareKeys); // I cant figure out how to use this but I want to use it since boundVec_ is mostly sorted every time
	    std::sort(boundVec_.begin(),boundVec_.end(), compareKeys);
	}
	/*! removes a reference copy (in the future this will be a pointer or reference) to a boundVec_ to easily access data stored in the csvData_ hashtable
	 * @param k the key that corresponds to what should have been recently removed from the hashtable 
	 */
	void removeFromBoundVec(const key & k)
	{
	   // presorted, since we sorted at add
	   auto it = std::lower_bound(boundVec_.begin(),boundVec_.end(),k,compareKeys);
	   boundVec_.erase(it);
	}
	
	/*! This is a mere procedure to shorten code for the setCell() methods
	 */
	inline void insertToCell(const key & k ,const value & val)
	{
	    const auto cIt = csvData_.find(k);
	    bool needToAddKeyToRefArr = true;
	    if (cIt != csvData_.end()) // if false, then key not found!
	    {
		csvData_.erase(cIt);
		needToAddKeyToRefArr =false;
	    }
		
	    csvData_.insert(mapEntry(k,val));
	    if (needToAddKeyToRefArr){
		addToBoundVec(k);
	    }
	}
    private:

	std::unordered_map<key,value, keyHasher, keyEqual > csvData_;
	/*! the boundVec_ exists in order to keep track and find keys in order */
	std::vector<key>		boundVec_; 
    };
}

#endif //fileguard
