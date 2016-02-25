#ifndef _DYNAMIC_STAGE_H_
#define _DYNAMIC_STAGE_H_

#include <typeinfo>
#include <memory>
#include <vector>

namespace sqeazy{

  template <typename raw_t>
  struct stage
  {

    typedef raw_t in_type;
    virtual ~stage() {}
    
    virtual std::string input_type() const {
      return typeid(raw_t).name();
    };
    
    virtual std::string output_type() const = 0;
    virtual std::string name() const = 0;
    virtual std::string config() const = 0;
    virtual bool is_compressor() const = 0;

    
    
  };
  
  /**
     \brief object that can filter the contents of incoming data, but will not play with their size in memory
     
     \param[in] 
     
     \return 
     \retval 
     
  */
  template <typename raw_t>
  struct filter : public stage<raw_t>
  {

    constexpr static bool is_compressor = false;
    typedef raw_t in_type;
    typedef raw_t out_type;

    filter(const std::string& _params = ""){}
    
    virtual ~filter() {}

    virtual out_type* encode(const in_type*, out_type*,std::vector<std::size_t>) {return nullptr;};
    virtual out_type* encode(const in_type* _in, out_type* _out,std::size_t len) {

      std::vector<std::size_t> shape(1,len);
      return encode(_in,_out,shape);

    };


    virtual int decode(const in_type*, out_type*,std::vector<std::size_t>) const {return 1;};
    virtual int decode(const in_type* _in, out_type* _out,std::size_t len) const {

      std::vector<std::size_t> shape(1,len);
      return decode(_in,_out,shape);

    };
    
    virtual std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const { return 0; };
  };

  /**
     \brief object that can alter the contents of incoming data, it will potentially play with their size
     
     \param[in] 
     
     \return 
     \retval 
     
  */
  template <typename raw_t, typename compressed_t = std::int8_t>
  struct sink : public stage<raw_t>
  {

    constexpr static bool is_compressor = true;
    typedef raw_t in_type;
    typedef compressed_t out_type;
    
    virtual ~sink() {}
    sink(const std::string& _params = ""){}
    
    /**
       \brief encode raw_t buffer _in of shape len and write results to _out
       
       \param[in] 
       
       \return out_type* pointer to the last+1 element of _out that was written (if error occurred nullptr is returned)
       \retval 
       
    */
    virtual out_type* encode(const raw_t* _in, out_type* _out,std::size_t len) {

      std::vector<std::size_t> shape(1,len);
      return encode(_in,_out,shape);

    };
    virtual out_type* encode(const raw_t*, out_type*,std::vector<std::size_t>) {return nullptr;};

    virtual int decode(const out_type* _in, raw_t* _out,std::size_t len) const {

      std::vector<std::size_t> shape(1,len);
      return decode(_in,_out,shape);

    };
    virtual int decode(const out_type*, raw_t*,std::vector<std::size_t>) const {return 1;};

    virtual std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const { return 0; };
  };    
    


  template <typename T>
  const std::shared_ptr<stage<typename T::in_type> > const_stage_view(const std::shared_ptr<T>& _in){
  // const stage<typename T::in_type>* const_stage_view(const std::shared_ptr<T>& _in){

    using return_t = stage<typename T::in_type>;
    
    auto value = std::dynamic_pointer_cast<return_t>(_in);// using return_t = const stage<typename T::in_type>*;
    
    // auto value = dynamic_cast<return_t>(_in.get());
    
    if(value)
      return value;
    else
      return nullptr;
  }

  template <typename T>
  std::shared_ptr<stage<typename T::in_type> > stage_view(std::shared_ptr<T>& _in){

    using return_t = stage<typename T::in_type>;
    
    auto value = std::dynamic_pointer_cast<return_t>(_in);
    
    if(value)
      return value;
    else
      return nullptr;
  }

  
  struct blank_filter : public  filter<void>
  {

    typedef void raw_t;
    ~blank_filter() {}
    blank_filter(const std::string& _params="") {}
    
    std::string input_type() const {return "";}
    std::string output_type() const {return "";}
    std::string name() const { return "blank";}
    std::string config() const { return "";}

    bool is_compressor() const {return false;}

    raw_t* encode(const raw_t* _in, raw_t* _out,std::vector<std::size_t> _shape) override final { return nullptr; };
    int decode(const raw_t* _in, raw_t* _out,std::vector<std::size_t> _shape) const override final { return 1; };
    
  };
  
  
  struct blank_sink : public  sink<void>
  {

    typedef void raw_t;
    ~blank_sink() {}
    blank_sink(const std::string& _params="") {}
    
    std::string input_type() const {return "";}
    std::string output_type() const {return "";}
    std::string name() const { return "blank";}
    std::string config() const { return "";}
    
    bool is_compressor() const {return false;}

    std::int8_t* encode(const raw_t* _in, std::int8_t* _out,std::vector<std::size_t> _shape) override final { return nullptr; };
    int decode(const std::int8_t* _in, raw_t* _out,std::vector<std::size_t> _shape) const override final { return 1; };
    
  };
  /**
     \brief method to create stage for a type given by a string
     
     \param[in] _name string to match type by
     
     \return 
     \retval 
     
  */
  template <typename pointee_t, typename Head>
  std::shared_ptr<pointee_t> create_by_name(const std::string &_name,
					    const std::string &_payload = "")
  {
    if(Head().name() == _name)
    {
      return std::dynamic_pointer_cast<pointee_t>(std::make_shared<Head>(_payload));
    }
    else
    {
      return nullptr;
    }
  }

  template <typename pointee_t, typename Head, typename Second, typename... Tail>
  std::shared_ptr<pointee_t> create_by_name(const std::string &_name,
					    const std::string &_payload = "")
  {
    if(Head().name() == _name)
    {
      return std::dynamic_pointer_cast<pointee_t>(std::make_shared<Head>(_payload));
    }
    else
    {
      return create_by_name<pointee_t, Second, Tail...>(_name,_payload);
    }
  }

  /**
     \brief method to check if list of types contains type given by runtime string name
     
     \param[in] _name string to match type by
     
     \return 
     \retval 
     
  */
  template <class Head>
  bool type_matches(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      return true;
    }
    else
    {
      return false;
    }
  }


  template <class Head, class Second, class... Tail>
  bool type_matches(const std::string &_name)
  {
    if(Head().name() == _name)
    {
      // got it
      return true;
    }
    else
    {
      return type_matches<Second, Tail...>(_name);
    }
  }

  /**
     \brief factory object that will create a stage defined by a runtime string
     
     \param[in] _name runtime string to define the stage
     
     \return 
     \retval 
     
  */
  template <typename... available_types>
  struct stage_factory
  {
    static_assert(sizeof...(available_types) > 0, "[dynamic_stage.hpp::stage_factory] Need at least one type for factory");

    template <typename pointee_t>
    static const std::shared_ptr<pointee_t> create(const std::string &_name,
					    const std::string &_payload = "")
    {
      // static_assert(sizeof...(available_types) > 0, "Need at least one type for factory");
      auto value = create_by_name<pointee_t,available_types...>(_name,_payload);
      
      return value;
    }

    static const bool has(const std::string &_name) { return type_matches<available_types...>(_name); }
  };



};

#endif /* _DYNAMIC_STAGE_H_ */
