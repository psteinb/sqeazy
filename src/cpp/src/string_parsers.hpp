#ifndef _STRING_PARSERS_H_
#define _STRING_PARSERS_H_


#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <boost/utility/string_ref.hpp>

#include <vector>
#include <map>
#include <string>


namespace sqeazy {

  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;
  namespace phoenix = boost::phoenix;

  typedef std::vector<std::string> vec_of_strings_t;
  typedef std::vector<boost::string_ref> vec_of_string_refs_t;
  
  typedef std::vector<std::pair<std::string, std::string> > vec_of_pairs_t;
  typedef std::map<std::string, std::string> parsed_map_t;

  static const std::pair<std::string, std::string> ignore_this_delimiters = std::make_pair("<verbatim>","</verbatim>");
  static const std::pair<boost::string_ref, boost::string_ref> ignore_this_delimiters_ref = std::make_pair(boost::string_ref(ignore_this_delimiters.first),
													   boost::string_ref(ignore_this_delimiters.second)); 
  
  template <typename char_itr_t>
  vec_of_strings_t split_by(char_itr_t _begin, char_itr_t _end, const std::string& _sep = ","
				    ){

    std::string allowed_characters = "a-zA-Z_0-9.=";
    auto f_itr = allowed_characters.find(_sep);

    if(f_itr != std::string::npos)
      allowed_characters.erase(f_itr,f_itr + _sep.size());
    
    vec_of_strings_t v;

    bool r = qi::parse( _begin,                          /*< start iterator >*/
			_end,                           /*< end iterator >*/
			(
			 +qi::char_(allowed_characters) % +qi::lit(_sep)
			 ),
			v
			);

    if (_begin != _end  && !r) // fail if we did not get a full match
      {
	std::string msg(_begin,_end);
	std::cerr << "[sqeazy::split_by] couldn't split anything in " << msg << "\n";
	v.clear();
	return v;
      }
    
    return v;

  }

  
  

  vec_of_strings_t split_string_by(const std::string& _data,
				   const std::string& _sep = ","
				   ){
    vec_of_strings_t value;

    if(_sep.empty() || _data.empty())
      return value;

    size_t approx_count = std::count(_data.begin(), _data.end(),_sep.front());
    value.reserve(approx_count);
    
    size_t first = 0;
    size_t last = first;

    while(first!=std::string::npos){

      //TODO: watch out for ignore_this
      first = _data.find(_sep,first);

      if(first!=std::string::npos)
	value.push_back(_data.substr(last,first-last));
      else{
	value.push_back(_data.substr(last));
	break;
      }
			
      last = (first += _sep.size());
      
    }
    
    value.reserve(value.size());
    return value;

  }

  

  vec_of_string_refs_t split_string_ref_to_ref(const boost::string_ref& _data,
					       const boost::string_ref& _sep = ","
					       ){
    vec_of_string_refs_t value;

    if(_sep.empty() || _data.empty())
      return value;

    std::size_t approx_count = std::count(_data.begin(), _data.end(),_sep.front());
    value.reserve(approx_count);
    
    std::size_t first = 0;

    boost::string_ref to_search(_data);
    std::size_t pos_of_ignore_delim = to_search.find(ignore_this_delimiters.first);
    
    while(to_search.size()){
	
      first = to_search.find_first_of(_sep);

      if(first != boost::string_ref::npos &&
	 pos_of_ignore_delim!= boost::string_ref::npos &&
	 pos_of_ignore_delim < first //did we pass a delimiter of ignorance?
	 ) 
	{
	pos_of_ignore_delim = to_search.find(ignore_this_delimiters.second);
	
	boost::string_ref tmp = to_search;
	tmp.remove_prefix(pos_of_ignore_delim+ignore_this_delimiters.second.size());
	first = tmp.find_first_of(_sep);
	if(first != boost::string_ref::npos)
	  first +=  pos_of_ignore_delim+ignore_this_delimiters.second.size();
	}	

      auto end_itr = first != boost::string_ref::npos ? to_search.begin()+first : to_search.end();
      std::size_t len = std::distance(to_search.begin(),end_itr);
      boost::string_ref result(&*to_search.begin(),
			       len);

      value.push_back(result);
      
      if(first==boost::string_ref::npos){
	break;
      }

      to_search.remove_prefix(first + _sep.size());
      pos_of_ignore_delim -= (first + _sep.size());
    }


    value.reserve(value.size());
    
    return value;

  }

  
  vec_of_strings_t split_string_ref_by(const boost::string_ref& _data,
				       const boost::string_ref& _sep = ","
				       ){
    

    auto tmp = split_string_ref_to_ref(_data,_sep);
    vec_of_strings_t value;
    value.reserve(tmp.size());
    
    for( const boost::string_ref& ref : tmp )
      value.push_back(std::string(ref.begin(), ref.end()));
    
    // if(_sep.empty() || _data.empty())
    //   return value;

    // std::size_t approx_count = std::count(_data.begin(), _data.end(),_sep.front());
    // value.reserve(approx_count);
    
    // std::size_t first = 0;

    // string_ref_t to_search(_data);
    // std::size_t pos_of_ignore_delim = to_search.find(ignore_this_delimiters.first);
    
    // while(to_search.size()){
	
    //   first = to_search.find_first_of(_sep);

    //   if(first != string_ref_t::npos &&
    // 	 pos_of_ignore_delim!= string_ref_t::npos &&
    // 	 pos_of_ignore_delim < first //did we pass a delimiter of ignorance?
    // 	 ) 
    // 	{
    // 	pos_of_ignore_delim = to_search.find(ignore_this_delimiters.second);
	
    // 	string_ref_t tmp = to_search;
    // 	tmp.remove_prefix(pos_of_ignore_delim+ignore_this_delimiters.second.size());
    // 	first = tmp.find_first_of(_sep);
    // 	if(first != string_ref_t::npos)
    // 	  first +=  pos_of_ignore_delim+ignore_this_delimiters.second.size();
    // 	}	

    //   auto end_itr = first != string_ref_t::npos ? to_search.begin()+first : to_search.end();
    //   std::string result(to_search.begin(),
    // 			 end_itr);

    //   value.push_back(result);
      
    //   if(first==string_ref_t::npos){
    // 	break;
    //   }

    //   to_search.remove_prefix(first + _sep.size());
    //   pos_of_ignore_delim -= (first + _sep.size());
    // }


    // value.reserve(value.size());
    
    return value;

  }
  

  

  
  
  template <typename Iterator>
    struct ordered_command_sequence 
      : qi::grammar<Iterator, vec_of_pairs_t()>
    {
      ordered_command_sequence()
          : ordered_command_sequence::base_type(query)
        {
	  query =  pair >> *(qi::lit("->") >> pair);
	  pair  =  key >> -('(' >> value >> ')');
	  key   =  qi::char_("a-zA-Z") >> *qi::char_("a-zA-Z_0-9");
	  //value =  +qi::char_("a-zA-Z_0-9:=,\"/");
	  value =  +(qi::print - qi::char_("()"));
        }

        qi::rule<Iterator, vec_of_pairs_t()> query;
        qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
        qi::rule<Iterator, std::string()> key, value;
    };
  
  template <typename char_itr_t>
  vec_of_pairs_t parse_by(char_itr_t _begin, char_itr_t _end, const std::string& _sep = ","){

    // ordered_command_sequence<std::string::iterator> p;
    ordered_command_sequence<char_itr_t> p;
    vec_of_pairs_t v;

    bool r = qi::parse(_begin, _end, p, v);
    if (_begin != _end  && !r) // fail if we did not get a full match
      {
	std::string msg(_begin,_end);
	std::cerr << "[sqeazy::parse_by] failed to parse " << msg << "\n";
	v.clear();
	return v;
      }

    return v;
  }
  
  template <typename char_itr_t>
  parsed_map_t unordered_parse_by(char_itr_t _begin, char_itr_t _end,
				  const std::string& _pair_sep = ",",
				  const std::string& _kv_sep = "="){

    using return_t = std::map<std::string,std::string>;

    return_t value;

    vec_of_strings_t parts = split_by(_begin,_end,_pair_sep);
    vec_of_strings_t key_value;
    
    for( auto & str : parts ){

      key_value = split_by(str.begin(),
			   str.end(),
			   _kv_sep);

      if(key_value.size()==1)
	value[key_value.front()] = "";
      
      if(key_value.size()>1 && key_value.size() % 2 == 0)
	value[key_value.front()] = key_value.back();

      key_value.clear();

    }
    
    return value;
    
  }

  parsed_map_t parse_string_by(const std::string& _payload,
			       const std::string& _pair_sep = ",",
			       const std::string& _kv_sep = "="){

    using return_t = std::map<std::string,std::string>;

    return_t value;

    vec_of_strings_t parts = split_string_by(_payload,_pair_sep);
    vec_of_strings_t key_value;
    
    for( auto & str : parts ){

      key_value = split_string_by(str,
				  _kv_sep);

      if(key_value.size()==1)
	value[key_value.front()] = "";
      
      if(key_value.size()>1 && key_value.size() % 2 == 0)
	value[key_value.front()] = key_value.back();

      key_value.clear();

    }
    
    return value;
    
  }



  
  struct pipeline_parser {

    typedef std::vector<boost::string_ref> vec_of_string_refs_t;
    
    vec_of_strings_t seperators_;
    vec_of_string_refs_t seperator_refs_;
    
    std::map<std::string, parsed_map_t> tree_; 

    pipeline_parser(const vec_of_strings_t& _seps = {"->",",","="}):
      seperators_(_seps),
      seperator_refs_(_seps.size()){

      std::uint8_t counter = 0;
      for(const std::string& str : _seps){
	seperator_refs_[counter++] = boost::string_ref(str);
      }
	
    }
    

    template <typename iter_t>
    std::map<std::string, parsed_map_t> operator()(iter_t _begin, iter_t _end){

      std::map<std::string, parsed_map_t> value;
      const std::size_t len = std::distance(_begin,_end);
      
      if(!len)
	return value;
      
      if(seperators_.empty())
	return value;

      
      boost::string_ref msg(&*_begin,len);
      
      vec_of_string_refs_t major_keys = split_string_ref_to_ref(msg,
								seperator_refs_.front());

      for(const boost::string_ref& maj_key : major_keys ){

	auto dist = maj_key.find("(");
	if(dist == std::string::npos)
	  dist = maj_key.size();
	
	std::string key(maj_key.begin(),
			maj_key.begin()+dist);
	
	parsed_map_t pmap;
	
	if( key.size() < maj_key.size() ){

	  boost::string_ref in_brackets(maj_key.begin()+dist+1,
					maj_key.size()-1-(dist+1));

	  vec_of_string_refs_t options = split_string_ref_to_ref(in_brackets,
								 seperator_refs_[1]);
	  for( const boost::string_ref& opt : options ){
	    vec_of_strings_t key_value = split_string_ref_by(opt,
							     seperator_refs_.back());

	    pmap[key_value.front()] = key_value.back();
	    
	  }
	  	  
	  
	}
	
	value[key] = pmap;
      }

      return value;
    }

  };
  
};

#endif /* _STRING_PARSERS_H_ */
