#pragma once

#ifdef SPACEFIGHT_EXPORTS
#define SPACEFIGHT_API __declspec(dllexport)
#else
#define SPACEFIGHT_API __declspec(dllimport)
#endif

#include <d2d1.h>
#include <random>

#pragma comment(lib, "d2d1.lib")

constexpr float scr_width{ 1000.0f };
constexpr float scr_height{ 800.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 750.0f };

constexpr int ERR_PTR{ 5001 };
constexpr int ERR_INDEX{ 5002 };
constexpr int ERR_PARAM{ 5003 };
constexpr int ERR_UNK{ 5004 };

enum class dirs { up = 0, down = 1, left = 2, right = 3, stop = 4 };
enum class actions { patrol = 0, move = 1, attack = 2 };

enum class creatures { fighter = 0, cruiser = 1, shuttle = 2, ship = 3, hero = 4,shot = 5 };
enum class background { intro = 0, field = 1 };
enum class assets { armor = 0, life = 1, shot = 2 };

namespace dll
{
	struct SPACEFIGHT_API FADING
	{
	private:
		float opacity{ 1.0f };
		int delay = 80;

	public:
		assets type{ assets::armor };
		D2D1_RECT_F rect{};
		
		float get_opacity()
		{
			--delay;
			if (delay <= 0)
			{
				delay = 80;
				opacity -= 0.1f;
			}

			return opacity;
		}
	};

	class SPACEFIGHT_API EXCEPTION
	{
	private:
		int _what{ ERR_UNK };

	public:
		EXCEPTION(int what_happened);

		const wchar_t* eGet() const;
	};

	class SPACEFIGHT_API RANDIT
	{
	private:
		std::mt19937* twister{ nullptr };

	public:
		RANDIT();
		~RANDIT();

		int operator()(int min, int max);
		float operator()(float min, float max);
	};

	template<typename T> class BAG
	{
	private:
		size_t max_size{ 1 };
		size_t next_pos{ 0 };

		T* main_ptr{ nullptr };

	public:
		BAG()
		{
			main_ptr = reinterpret_cast<T*>(calloc(max_size, sizeof(T)));
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
		}
		BAG(size_t lenght)
		{
			max_size = lenght;
			main_ptr = reinterpret_cast<T*>(calloc(lenght, sizeof(T)));
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
		}
		BAG(BAG& other)
		{
			if (!other.main_ptr)throw EXCEPTION(ERR_PTR);

			free(main_ptr);

			max_size = other.max_size;
			next_pos = other.next_pos;

			main_ptr = reinterpret_cast<T*>(calloc(max_size, sizeof(T)));

			if (!main_ptr)throw EXCEPTION(ERR_PTR);

			if (next_pos > 0)for (size_t i = 0; i < next_pos; ++i)main_ptr[i] = other.main_ptr[i];
		}
		BAG(BAG&& other)
		{
			if (!other.main_ptr)throw EXCEPTION(ERR_PTR);

			free(main_ptr);

			max_size = other.max_size;
			next_pos = other.next_pos;
			main_ptr = other.main_ptr;

			other.main_ptr = nullptr;
		}

		~BAG()
		{
			free(main_ptr);
		}

		size_t size() const
		{
			return next_pos;
		}
		size_t capacity() const
		{
			return max_size;
		}

		bool empty()const
		{
			return(next_pos == 0);
		}

		void clear()
		{
			free(main_ptr);

			main_ptr = reinterpret_cast<T*>(calloc(1, sizeof(T)));
			max_size = 1;
			next_pos = 0;

			if (!main_ptr)throw EXCEPTION(ERR_PTR);
		}

		T& operator[](size_t index)
		{
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
			else if (index >= next_pos)return main_ptr[next_pos - 1];
			
			return main_ptr[index];
		}

		BAG& operator=(BAG& other)
		{
			if (!other.main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				free(main_ptr);

				max_size = other.max_size;
				next_pos = other.next_pos;

				main_ptr = reinterpret_cast<T*>(calloc(max_size, sizeof(T)));
				if (!main_ptr)throw EXCEPTION(ERR_PTR);
				else  if (next_pos > 0)for (size_t i = 0; i < next_pos; ++i)main_ptr[i] = other.main_ptr[i];
			}

			return *this;
		}
		BAG& operator=(BAG&& other)
		{
			if (!other.main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				free(main_ptr);

				main_ptr = other.main_ptr;
				max_size = other.max_size;
				next_pos = other.next_pos;

				other.main_ptr = nullptr;
			}

			return *this;
		}

		void push_back(T element)
		{
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				if (next_pos + 1 <= max_size)
				{
					main_ptr[next_pos] = element;
					++next_pos;
				}
				else
				{
					++max_size;
					T* temp_ptr = reinterpret_cast<T*>(realloc(main_ptr, max_size * sizeof(T)));

					if (!temp_ptr)throw EXCEPTION(ERR_PTR);
					else
					{
						main_ptr = temp_ptr;
						temp_ptr = nullptr;

						main_ptr[next_pos] = element;
						++next_pos;
					}
				}
			}
		}
		void push_back(T* element)
		{
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				if (next_pos + 1 <= max_size)
				{
					main_ptr[next_pos] = *element;
					++next_pos;
				}
				else
				{
					++max_size;
					T* temp_ptr = reinterpret_cast<T*>(realloc(main_ptr, max_size * sizeof(T)));

					if (!temp_ptr)throw EXCEPTION(ERR_PTR);
					else
					{
						main_ptr = temp_ptr;
						temp_ptr = nullptr;

						main_ptr[next_pos] = *element;
						++next_pos;
					}
				}
			}
		}

		void insert(T element, size_t index)
		{
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				if (index >= next_pos)throw EXCEPTION(ERR_INDEX);
				else
				{
					if (next_pos + 1 <= max_size)
					{
						for (size_t i = next_pos; i > index; --i)main_ptr[index] = main_ptr[index - 1];
						main_ptr[index] = element;
						++next_pos;
					}
					else
					{
						++max_size;
						T* temp_ptr = reinterpret_cast<T*>(realloc(main_ptr, max_size * sizeof(T)));
						if (!temp_ptr)throw EXCEPTION(ERR_PTR);
						else
						{
							main_ptr = temp_ptr;
							temp_ptr = nullptr;

							for (size_t i = next_pos; i > index; --i)main_ptr[index] = main_ptr[index - 1];
							main_ptr[index] = element;
							++next_pos;
						}
					}
				}
			}
		}
		void insert(T* element, size_t index)
		{
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				if (index >= next_pos)throw EXCEPTION(ERR_INDEX);
				else
				{
					if (next_pos + 1 <= max_size)
					{
						for (size_t i = next_pos; i > index; --i)main_ptr[index] = main_ptr[index - 1];
						main_ptr[index] = *element;
						++next_pos;
					}
					else
					{
						++max_size;
						T* temp_ptr = reinterpret_cast<T*>(realloc(main_ptr, max_size * sizeof(T)));
						if (!temp_ptr)throw EXCEPTION(ERR_PTR);
						else
						{
							main_ptr = temp_ptr;
							temp_ptr = nullptr;

							for (size_t i = next_pos; i > index; --i)main_ptr[index] = main_ptr[index - 1];
							main_ptr[index] = *element;
							++next_pos;
						}
					}
				}
			}
		}
		void erase(size_t index)
		{
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				if (next_pos < 2)clear();
				else
				{
					if (index >= next_pos)throw EXCEPTION(ERR_INDEX);
					else
					{
						for (size_t i = index; i < next_pos - 1; ++i)main_ptr[index] = main_ptr[index + 1];
						--next_pos;
					}	
				}
			}
		}
	
		void push_front(T element)
		{
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				if (empty())*main_ptr = element;
				else
				{
					if (next_pos + 1 <= max_size)
					{
						for (size_t index = next_pos; index > 0; --index)main_ptr[index] = main_ptr[index - 1];
						*main_ptr = element;
						++next_pos;
					}
					else
					{
						++max_size;
						T* temp_ptr = reinterpret_cast<T*>(realloc(main_ptr, max_size * sizeof(T)));
						if (!temp_ptr)throw EXCEPTION(ERR_PTR);
						else
						{
							main_ptr = temp_ptr;
							temp_ptr = nullptr;
							for (size_t index = next_pos; index > 0; --index)main_ptr[index] = main_ptr[index - 1];
							*main_ptr = element;
							++next_pos;
						}
					}
				}
			}
		}
		void push_front(T* element)
		{
			if (!main_ptr)throw EXCEPTION(ERR_PTR);
			else
			{
				if (empty())*main_ptr = element;
				else
				{
					if (next_pos + 1 <= max_size)
					{
						for (size_t index = next_pos; index > 0; --index)main_ptr[index] = main_ptr[index - 1];
						*main_ptr = *element;
						++next_pos;
					}
					else
					{
						++max_size;
						T* temp_ptr = reinterpret_cast<T*>(realloc(main_ptr, max_size * sizeof(T)));
						if (!temp_ptr)throw EXCEPTION(ERR_PTR);
						else
						{
							main_ptr = temp_ptr;
							temp_ptr = nullptr;
							for (size_t index = next_pos; index > 0; --index)main_ptr[index] = main_ptr[index - 1];
							*main_ptr = *element;
							++next_pos;
						}
					}
				}
			}
		}

		T& front()
		{
			if (empty())throw EXCEPTION(ERR_INDEX);

			return *main_ptr;
		}
		T& back()
		{
			if (empty())throw EXCEPTION(ERR_INDEX);

			return main_ptr[next_pos - 1];
		}
	};

	class SPACEFIGHT_API PROTON
	{
	protected:
		float _width{ 0 };
		float _height{ 0 };

		bool in_heap{ false };

	public:
		D2D1_POINT_2F start{};
		D2D1_POINT_2F end{};
		D2D1_POINT_2F center{};

		D2D1_RECT_F my_rect{};

		float x_rad{ 0 };
		float y_rad{ 0 };

		PROTON();
		PROTON(D2D1_POINT_2F start_point);
		PROTON(D2D1_POINT_2F start_point, float first_width, float first_height);
		PROTON(float first_x, float first_y, float first_width, float first_height);

		virtual ~PROTON();

		float get_width() const;
		float get_height() const;

		void set_edges();

		void set_width(float new_width);
		void set_height(float new_height);
		void new_dims(float new_width, float new_height);
	
		static PROTON* create(float first_x, float first_y, float first_width, float first_height);
	};

	class SPACEFIGHT_API BACKGROUND: public PROTON
	{
	private:
		int current_frame = 0;
		int max_frames = 0;
		int frame_delay = 1;
		int max_frame_delay = 1;

	public:
		background type = background::intro;

		BACKGROUND(background what);

		int frame();

	};

	class SPACEFIGHT_API CREATURES :public PROTON
	{
	private:
		int current_frame{ 0 };
		int max_frames{ 0 };
		int frame_delay{ 0 };
		int max_frame_delay{ 0 };

		int attack_delay{ 0 };
		int max_attack_delay{ 0 };
		
		float speed{ 1.0f };
		float view_range{ 0 };

		float move_sx{ 0 };
		float move_sy{ 0 };
		float move_ex{ 0 };
		float move_ey{ 0 };

		float slope{ 0 };
		float intercept{ 0 };

		bool hor_dir{ false };
		bool ver_dir{ false };

		CREATURES(creatures _what, float _sx, float _sy);
		
	public:
		creatures type{ creatures::hero };
		actions action = actions::patrol;
		
		dirs dir{ dirs::stop };

		bool hero_moving{ false };

		int lifes = 0;
		int strenght{ 0 };
		int armor{ 0 };
		
		float angle{ 0 };

		void set_path(float targ_x, float targ_y);

		void move(float gear);

		bool shot_move(float gear);
		
		int attack();

		int get_frame();

		float get_view_range() const;

		float get_move_target_x() const;

		float get_move_target_y() const;

		void Release();

		float rotate_angle(float oppos, float adjanced);
	
		static CREATURES* create(creatures what, float sx, float sy);
	};

	// FUNCTIONS **********************************

	float SPACEFIGHT_API distance(D2D1_POINT_2F first, D2D1_POINT_2F second);

	bool SPACEFIGHT_API intersect(D2D1_RECT_F first, D2D1_RECT_F second);

	bool SPACEFIGHT_API intersect(D2D1_POINT_2F first_center, D2D1_POINT_2F second_center,
		float first_xrad, float second_xrad, float first_yrad, float second_yrad);

	void SPACEFIGHT_API sort(BAG<D2D1_POINT_2F>& bag, D2D1_POINT_2F target);

	void SPACEFIGHT_API AIMove(CREATURES*& evil, BAG<D2D1_POINT_2F>& assets_centeres, BAG<D2D1_POINT_2F>& meteor_centeres,
		D2D1_POINT_2F hero_center);
}