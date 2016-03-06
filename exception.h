#pragma once
#include <string.h>

/* Start of 'hcp' namespace */
namespace hcp
{
  /* Start of 'exc' namespace */
  namespace exc
  {
    /* Exception class */
    class Exception
    {
    private:
      const char *str_;
      bool doFree_;

      /* Free memory if it need internal function
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      inline void _freeMem( void )
      {
        if (doFree_ == true)
          delete[] str_;
        str_ = nullptr;
        doFree_ = false;
      } /* End of 'freeMem' function */

      /* Copy string to class function
       *   assumes that 'str_' is already free
       * ARGUMENTS:
       *   - string to copy
       *       const char *str;
       * RETURNS: None.
       */
      inline void _copyStr( const char *str )
      {
        if (str != nullptr)
        {
          size_t len = strlen(str) + 1;
          str_ = new char[len];
          if (str_ != nullptr)
          {
            memcpy(const_cast<char *>(str_), str, len);
            doFree_ = true;
          }
        }
      } /* End of 'copyStr' function */

    public:
      /* 'Exception' class constructor
       * ARGUMENTS: None.
       */
      inline Exception( void ) : str_(nullptr), doFree_(false)
      {
      } /* End of 'Exception' class constructor */

      /* 'Exception' class constructor
       * ARGUMENTS:
       *   - pointer to string
       *       const char *str;
       */
      inline Exception( const char *str ) : str_(nullptr), doFree_(false)
      {
        _freeMem();
        _copyStr(str);
      } /* End of 'Exception' class constructor */

      /* 'Exception' class constructor
       * ARGUMENTS:
       *   - pointer to static string
       *       const char *str;
       *   - indicates that previous is static string
       *       int;
       */
      inline Exception( const char *str, int ) : str_(str), doFree_(false) 
      {
      } /* End of 'Exception' class constructor */

      /* 'Exception' class constructor
       * ARGUMENTS:
       *   - reference to variable to copy
       *       Exception& ex;
       */
      inline Exception( const Exception &ex ) : str_(nullptr), doFree_(false)
      {
        *this = ex;
      } /* End of 'Exception' class constructor */

      /* Overload of assignment operator function
       * ARGUMENTS:
       *   - reference to variable to assign
       *       const Exception &ex;
       * RETURNS:
       *   - self-reference
       *       (Exception&);
       */
      inline Exception & operator =( const Exception &ex )
      {
        if (this != &ex)
        {
          _freeMem();
          if (ex.doFree_ == true)
            _copyStr(ex.str_);
          else
            str_ = ex.str_;
        }

        return *this;
      } /* End of 'operator =' function */

      /* 'Exception' class destructor
       */
      inline virtual ~Exception( void )
      {
        _freeMem();
      } /* End of 'Exception' class destructor */

      /* Get exception string function
       * ARGUMENTS: None.
       * RETURNS:
       *   - pointer to exception string
       *       (const char *);
       */
      inline const char * get( void )
      {
        return (str_ != nullptr ? str_ : "undefined exception");
      } /* End of 'get' function */
    }; /* End of 'Exception' class */

    class BadAlloc : public Exception
    {
    public:
      BadAlloc( const char *str ) : Exception(str)
      {
      }

      BadAlloc( void ) : Exception("bad alloc", 1)
      {
      }
    };

    class InvalidParameter : public Exception
    {
    public:
      InvalidParameter( const char *str ) : Exception(str)
      {
      }

      InvalidParameter( void ) : Exception("invalid parameter", 1)
      {
      }

      InvalidParameter( const char *str, int ) : Exception(str, 1)
      {
      }
    };

    class OutOfRange : public InvalidParameter
    {
    public:
      OutOfRange( const char *str ) : InvalidParameter(str)
      {
      }

      OutOfRange( void ) : InvalidParameter("out of range", 1)
      {
      }
    };

    class InvalidData : public Exception
    {
    public:
      InvalidData( const char *str ) : Exception(str)
      {
      }

      InvalidData( void ) : Exception("invalid data", 1)
      {
      }

      InvalidData( const char *str, int ) : Exception(str, 1)
      {
      }
    };

    class ImpossibleOperation : public Exception
    {
    public:
      ImpossibleOperation( const char *str ) : Exception(str)
      {
      }

      ImpossibleOperation( void ) : Exception("impossible operation", 1)
      {
      }

      ImpossibleOperation( const char *str, int ) : Exception(str, 1)
      {
      }
    };

    class OverFlow : public ImpossibleOperation
    {
    public:
      OverFlow( const char *str ) : ImpossibleOperation(str)
      {
      }

      OverFlow( void ) : ImpossibleOperation("overflow", 1)
      {
      }
    };

    class UnderFlow : public ImpossibleOperation
    {
    public:
      UnderFlow( const char *str ) : ImpossibleOperation(str)
      {
      }

      UnderFlow( void ) : ImpossibleOperation("overflow", 1)
      {
      }
    };

    class ZeroDivision : public InvalidParameter
    {
    public:
      ZeroDivision( void ) : InvalidParameter("zero division", 1)
      {
      }

      ZeroDivision( const char *str ) : InvalidParameter(str)
      {
      }
    };

    class IOError : public InvalidData
    {
    public:
      IOError( void ) : InvalidData("input/output error", 1)
      {
      }

      IOError( const char *str ) : InvalidData(str)
      {
      }

      IOError( const char *str, int ) : InvalidData(str, 1)
      {
      }
    };

    class InputError : public IOError
    {
    public:
      InputError( void ) : IOError("input error", 1)
      {
      }

      InputError( const char *str ) : IOError(str)
      {
      }
    };

    class OutputError : public IOError
    {
    public:
      OutputError( void ) : IOError("output error", 1)
      {
      }

      OutputError( const char *str ) : IOError(str)
      {
      }
    };

    class AlreadyExist : public InvalidParameter
    {
    public:
      AlreadyExist( void ) : InvalidParameter("already exist", 1)
      {
      }

      AlreadyExist( const char *str ) : InvalidParameter(str)
      {
      }
    };

    class LackOfData : public ImpossibleOperation
    {
    public:
      LackOfData( void ) : ImpossibleOperation("lack of data", 1)
      {
      }

      LackOfData( const char *str ) : ImpossibleOperation(str)
      {
      }
    };

    class DataNotFound : public InvalidParameter
    {
    public:
      DataNotFound( void ) : InvalidParameter("data not found", 1)
      {
      }

      DataNotFound( const char *str ) : InvalidParameter(str)
      {
      }
    };
    
    class EndOfFile : public Exception
    {
    public:
      EndOfFile( void ) : Exception("end of file", 1)
      {
      }

      EndOfFile( const char *str ) : Exception(str)
      {
      }
    };
  } /* End of 'exc' namespace */
} /* End of 'hcp' namespace */