// -*- mode: c++; indent-tabs-mode: nil; -*-
//
// Manta
// Copyright (c) 2013 Illumina, Inc.
//
// This software is provided under the terms and conditions of the
// Illumina Open Source Software License 1.
//
// You should have received a copy of the Illumina Open Source
// Software License 1 along with this program. If not, see
// <https://github.com/downloads/sequencing/licenses/>.
//

///
/// \author Bret Barnes
///

#include "blt_util/samtools_fasta_util.hh"

#include "blt_util/blt_exception.hh"
#include "blt_util/parse_util.hh"
#include "blt_util/seq_util.hh"
#include "blt_util/string_util.hh"

extern "C"
{
#include "faidx.h"
}

#include <cassert>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>




void
get_chrom_sizes(const std::string& fai_file,
                std::map<std::string,unsigned>& chrom_sizes)
{

    static const char delim('\t');

    chrom_sizes.clear();
    std::ifstream fis(fai_file.c_str());

    std::string line;
    std::vector<std::string> word;

    while (! fis.eof())
    {
        getline(fis, line);

        split_string(line, delim, word);

        assert(2 <= word.size());

        assert(0 == chrom_sizes.count(word[0]));

        const unsigned length(illumina::blt_util::parse_unsigned_str(word[1]));
        chrom_sizes.insert(std::make_pair(word[0],length));
    }
}



unsigned
get_chrom_length(const std::string& fai_file,
                 const std::string& chrom_name)
{

    static const char delim('\t');

    bool isFound(false);
    unsigned retval(0);
    {
        std::ifstream fis(fai_file.c_str());

        std::string line;
        std::vector<std::string> word;

        while (! fis.eof())
        {
            getline(fis, line);

            split_string(line, delim, word);

            assert(2 <= word.size());
            if (word[0] != chrom_name) continue;
            retval=illumina::blt_util::parse_unsigned_str(word[1]);
            isFound=true;
            break;
        }
    }

    if (! isFound)
    {
        std::ostringstream oss;
        oss << "ERROR: Unable to find chromosome '" << chrom_name << "' in fai file '" << fai_file << "'\n";
        throw blt_exception(oss.str().c_str());
    }
    return retval;
}



void
get_region_seq(const std::string& ref_file,
               const std::string& fa_region,
               std::string& ref_seq)
{

    faidx_t* fai(fai_load(ref_file.c_str()));
    int len; // throwaway...
    std::auto_ptr<char> ref_tmp(fai_fetch(fai,fa_region.c_str(), &len));
    if (NULL == ref_tmp.get())
    {
        std::ostringstream oss;
        oss << "ERROR: Can't find sequence region '" << fa_region << "' in reference file: '" << ref_file << "'\n";
        throw blt_exception(oss.str().c_str());
    }
    ref_seq.assign(ref_tmp.get());
    fai_destroy(fai);
}



void
get_region_seq(const std::string& ref_file,
               const std::string& chrom,
               const int begin_pos,
               const int end_pos,
               std::string& ref_seq)
{
    faidx_t* fai(fai_load(ref_file.c_str()));
    int len; // throwaway...
    std::auto_ptr<char> ref_tmp(faidx_fetch_seq(fai,const_cast<char*>(chrom.c_str()), begin_pos, end_pos, &len));
    if (NULL == ref_tmp.get())
    {
        std::ostringstream oss;
        oss << "ERROR: Can't find sequence region '" << chrom << ":" << (begin_pos+1) << "-" << (end_pos+1) << "' in reference file: '" << ref_file << "'\n";
        throw blt_exception(oss.str().c_str());
    }
    ref_seq.assign(ref_tmp.get());
    fai_destroy(fai);
}



void
get_standardized_region_seq(
    const std::string& ref_file,
    const std::string& chrom,
    const int begin_pos,
    const int end_pos,
    std::string& ref_seq)
{
    get_region_seq(ref_file,chrom,begin_pos,end_pos,ref_seq);
    standardize_ref_seq(ref_file.c_str(), chrom.c_str(), ref_seq, begin_pos);
}
