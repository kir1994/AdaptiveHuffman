#pragma once
#include <sstream>

#include "..\commondf.h"

namespace hcp
{
  /* Convert file size in bytes to string function
   * ARGUMENTS:
   *   - size in bytes
   *       T sizeInBytes;
   *   - precision of file size
   *       UINT prec;
   * RETURNS:
   *   - string with file size
   *       (std::string);
   */
  template< class T >
  std::string fileSizeToString( T sizeInBytes, UINT prec )
  {
    INT state = 0;
    LDBL size = (LDBL)sizeInBytes;
    while (size >= 1024.)
    {
      size /= 1024.;
      ++state;
    }
    std::stringstream ss;
    ss.setf(ios::fixed);
    ss.precision(prec);
    ss << size << " " << 
          (state == 0 ? "B" : (state == 1 ? "KB" : (state == 2 ? "MB" : (state == 3 ? "GB" : "??"))));
    return ss.str();
  } /* End of 'fileSizeToString' function */
} /* End of 'hcp' namespace */