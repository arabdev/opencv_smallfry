#ifndef __opencv_db_onboard__
#define __opencv_db_onboard__

#include "opencv2/core/core.hpp"

struct opencv_db 
{
    //! host,user,pw,(db even) might be 0 in case of a local connection(sqlite,slimxml,memdb)
    virtual bool open( const char * db, const char * host, const char * user, const char * pw )    { return false; }
    virtual bool close() { return false; }

    //! container for several cv::Mat's whithin the db:
    virtual bool create( const char * table ) { return false; }
    virtual bool drop  ( const char * table ) { return false; }

    //! serialize cv::Mat (key-value style)
    virtual bool write( const char * table, const char * name, const cv::Mat & mat ) { return false; }
    virtual bool read ( const char * table, const char * name, cv::Mat & mat )       { return false; }

    //! one-way (it does not return results), native, administrative (sql or js) statements
    virtual bool exec( const char * statement )      { return false; }

    virtual ~opencv_db() {} // we're a base class
};

#endif // __opencv_db_onboard__

