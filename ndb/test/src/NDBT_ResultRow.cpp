/* Copyright (C) 2003 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <ndb_global.h>
#include "NDBT_ResultRow.hpp"
#include <NdbOut.hpp>
#include <NdbSchemaCon.hpp>

NDBT_ResultRow::NDBT_ResultRow(const NdbDictionary::Table& tab,
			       char attrib_delimiter)
  : m_table(tab)
{
  assert(tab.getObjectStatus() == NdbDictionary::Object::Retrieved);

  cols = tab.getNoOfColumns();
  names = new char *       [cols];
  data  = new NdbRecAttr * [cols];
  
  for(int i = 0; i<cols; i++){
    names[i] = new char[255];
    strcpy(names[i], tab.getColumn(i)->getName());
  }  

  ad[0] = attrib_delimiter;
  ad[1] = 0;
  m_ownData = false;
}

NDBT_ResultRow::~NDBT_ResultRow(){
  for(int i = 0; i<cols; i++){
    delete [] names[i];
  }  
  delete [] names;

  if(m_ownData){
    for(int i = 0; i<cols; i++)
      delete data[i];
  }
  delete [] data;
}

NdbRecAttr* & 
NDBT_ResultRow::attributeStore(int i){

  return data[i];
}


const 
NdbRecAttr * 
NDBT_ResultRow::attributeStore(const char* name){
  for(int i = 0; i<cols; i++){
    if (strcmp(names[i], name) == 0)
      return data[i];
  }  
  assert(false);
  return 0;
}

NdbOut & 
NDBT_ResultRow::header (NdbOut & out) const {
  for(int i = 0; i<cols; i++){
    out << names[i];
    if (i < cols-1)
      out << ad;
  }
  return out;
}

BaseString NDBT_ResultRow::c_str() {
  
  BaseString str;
  
  char buf[10];
  for(int i = 0; i<cols; i++){
    if(data[i]->isNULL()){
      sprintf(buf, "NULL");
      str.append(buf);    
    }else{
      Uint32* p = (Uint32*)data[i]->aRef();
      Uint32 sizeInBytes = data[i]->attrSize() * data[i]->arraySize();
      for (Uint32 j = 0; j < sizeInBytes; j+=(sizeof(Uint32))){
	str.append("H'");
	sprintf(buf, "%.8x", *p);
	p++;
	str.append(buf);
	if ((j + sizeof(Uint32)) < sizeInBytes)
	  str.append(", ");
      }
    }
    str.append("\n");
  }
  str.append("*");
  
  //ndbout << "NDBT_ResultRow::c_str() = " << str.c_str() << endl;
  
  return str;
}

NdbOut & 
operator << (NdbOut& ndbout, const NDBT_ResultRow & res) {
  for(int i = 0; i<res.cols; i++)
    ndbout << *(res.data[i]) << "\t";
  return ndbout;
}

NDBT_ResultRow *
NDBT_ResultRow::clone () const {

  NDBT_ResultRow * row = new NDBT_ResultRow(m_table, ad[0]);
  row->m_ownData = true;
  Uint32 noOfColumns = m_table.getNoOfColumns();
  for(Uint32 i = 0; i < noOfColumns; i++){
    row->data[i] = data[i]->clone();
  }
  
  return row;
}
