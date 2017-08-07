#pragma once
class Utility
{
public:
	// Non-copyable
	Utility(Utility const&) = delete;
	void operator=(Utility const&) = delete;

private:
	Utility();
	~Utility();
};

