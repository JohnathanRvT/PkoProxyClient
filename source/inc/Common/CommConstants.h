#pragma once

namespace CommConstants {
class Login {
public:
	static constexpr bool UsernameLengthOK(size_t len) { return len <= username_max_length && len >= username_min_length; };
	static constexpr bool PasswordLengthOK(size_t len) { return len <= password_max_length && len >= password_min_length; };

private:
	static constexpr auto username_min_length{4};
	static constexpr auto username_max_length{14};

	static constexpr auto password_min_length{4};
	static constexpr auto password_max_length{14};
};

class Character {
public:
	static constexpr auto name_max_length{16};
};

class Guild {
public:
	static constexpr auto name_max_length{16};
	static constexpr auto password_max_length{16};
};

class Chat {
public:
	static constexpr auto groupname_max_length{16};
};

}; // namespace CommConstants
