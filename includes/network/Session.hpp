#ifndef _SESSION_HPP_
#define _SESSION_HPP_

#include <iostream>
#include <ctime>
#include <map>
#include <bits/stdc++.h>

class Session
{
	private:
		std::string _the_id;
		std::string _id;
		std::map<std::string, std::string> _data;
		std::time_t _lastAccessTime;
	public:
		Session();
		Session(const std::string &id);
		Session(const Session &other);
		Session &operator=(const Session &other);
		~Session();

		static std::string generateId();
		const std::string &getId() const;
		const std::string &getTheId() const;
		void setData(const std::string &key, const std::string &value);
		const std::map<std::string, std::string> &getAllData() const;
		std::time_t getLastAccessTime() const;
		void updateLastAccessTime();
};

#endif