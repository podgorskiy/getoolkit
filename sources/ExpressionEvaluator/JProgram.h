#pragma once
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <list>

namespace ExpessionEvaluator
{
	void hexDump(char *desc, void *addr, int len)
	{
		int i;
		unsigned char buff[17];
		unsigned char *pc = (unsigned char*)addr;

		// Output description if given.
		if (desc != NULL)
			printf ("%s:\n", desc);

		// Process every byte in the data.
		for (i = 0; i < len; i++) {
			// Multiple of 16 means new line (with line offset).

			if ((i % 16) == 0) {
				// Just don't print ASCII for the zeroth line.
				if (i != 0)
					printf("  %s\n", buff);

				// Output the offset.
				printf("  %04x ", i);
			}

			// Now the hex code for the specific character.
			printf(" %02x", pc[i]);

			// And store a printable ASCII character for later.
			if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
				buff[i % 16] = '.';
			} else {
				buff[i % 16] = pc[i];
			}

			buff[(i % 16) + 1] = '\0';
		}

		// Pad out last line if not exactly 16 characters.
		while ((i % 16) != 0) {
			printf("   ");
			i++;
		}

		// And print the final ASCII bit.
		printf("  %s\n", buff);
	}

	struct JitProgram
	{
		JitProgram(): pointer(0), idc(0), size(0)
		{
			size_t page_size_multiple = sysconf(_SC_PAGE_SIZE);
			mem = (uint8_t*) mmap(NULL, page_size_multiple * 8, PROT_READ | PROT_WRITE | PROT_EXEC,
			                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if (mem == MAP_FAILED)
			{
				std::cerr << "Can't allocate memory\n";
				std::exit(1);
			}
			pointer = 0;
		}

		enum IC: uint64_t
		{
			ic_movabs_rax,
			ic_mov_mrax_xmm,
			ic_mov_rax_xmm,
			ic_mov_xmm_rax,
			ic_add_xmm1_xmm0,
			ic_sub_xmm1_xmm0,
			ic_mul_xmm1_xmm0,
			ic_div_xmm1_xmm0,
			ic_pxor_xmm1_xmm0,
			ic_push_rax,
			ic_pop_rax,
			ic_push_mrax,
			ic_call_rax,
			ic_ret,
			ic_address,
			ic_im_value,
		};

		void PrintIntermidiate()
		{
			auto it = m_intermidiate_code.begin();
			while (it != m_intermidiate_code.end())
			{
				uint64_t op = *it;
				switch(op)
				{
					case ic_movabs_rax: printf("ic_movabs_rax\n"); break;
					case ic_im_value: printf("ic_im_value\n"); ++it; break;
					case ic_address: printf("ic_address\n"); ++it; break;
					case ic_mov_mrax_xmm: printf("ic_mov_mrax_xmm %d\n", (int)*++it);break;
					case ic_mov_rax_xmm: printf("ic_mov_rax_xmm %d\n", (int)*++it);break;
					case ic_mov_xmm_rax: printf("ic_mov_xmm_rax %d\n", (int)*++it);break;
					case ic_add_xmm1_xmm0: printf("ic_add_xmm1_xmm0\n"); break;
					case ic_sub_xmm1_xmm0: printf("ic_sub_xmm1_xmm0\n"); break;
					case ic_mul_xmm1_xmm0: printf("ic_mul_xmm1_xmm0\n"); break;
					case ic_div_xmm1_xmm0: printf("ic_div_xmm1_xmm0\n"); break;
					case ic_pxor_xmm1_xmm0: printf("ic_pxor_xmm1_xmm0\n"); break;
					case ic_push_rax: printf("ic_push_rax\n"); break;
					case ic_pop_rax: printf("ic_pop_rax\n"); break;
					case ic_push_mrax: printf("ic_push_mrax\n"); break;
					case ic_call_rax: printf("ic_call_rax\n"); break;
					case ic_ret: printf("ic_ret\n"); break;
					default: printf("ERROR\n"); break;
				}
				++it;
			}
		}

		void Translate()
		{
			Optimize();
			auto it = m_intermidiate_code.begin();
			while (it != m_intermidiate_code.end())
			{
				uint64_t op = *it;
				switch(op)
				{
					case ic_movabs_rax:
					{
						Write((uint16_t) 0xb848);
						break;
					}
					case ic_im_value:
					{
						++it;
						uint64_t x = *it;
						Write(x);
						break;
					}
					case ic_address:
					{
						relocations_pointer.push_back(pointer);
						++it;
						uint64_t x = *it;
						Write(x);
						break;
					}
					case ic_mov_mrax_xmm:
						{
							++it;
							uint8_t x = *it;
							Write((uint32_t) (0x00100ff2 + 0x08000000 * x));
						}
						break;
					case ic_mov_rax_xmm:
						{
							++it;
							uint8_t x = *it;
							Write((uint32_t) 0x6e0f4866);
							Write((uint8_t) (0xc0 + 8 * x));
						}
						break;
					case ic_mov_xmm_rax:
						{
							++it;
							uint8_t x = *it;
							Write((uint32_t) 0x7e0f4866);
							Write((uint8_t) (0xc0 + 8 * x));
						}
						break;
					case ic_add_xmm1_xmm0:
						Write((uint32_t) 0xc1580ff2);
						break;
					case ic_sub_xmm1_xmm0:
						Write((uint32_t) 0xc15c0ff2);
						break;
					case ic_mul_xmm1_xmm0:
						Write((uint32_t) 0xc1590ff2);
						break;
					case ic_div_xmm1_xmm0:
						Write((uint32_t) 0xc15e0ff2);
						break;
					case ic_pxor_xmm1_xmm0:
						Write((uint32_t) 0xc1ef0f66);
						break;
					case ic_push_rax:
						Write((uint8_t) 0x50);
						break;
					case ic_pop_rax:
						Write((uint8_t) 0x58);
						break;
					case ic_push_mrax:
						Write((uint16_t) 0x30ff);
						break;
					case ic_call_rax:
						Write((uint16_t) 0xd0ff);
						break;
					case ic_ret:
						Write((uint8_t) 0xc3);
						break;
					default:
						assert(op != 0);
						break;
				}
				++it;
			}
			m_intermidiate_code.clear();
			Eval = ((double (*)()) mem);
		}

		void Optimize()
		{
			//printf("Before:\n");
			//PrintIntermidiate();
			//printf("\n");
			std::list<uint64_t> current_code;
			std::list<uint64_t> optimized_code;

			current_code = m_intermidiate_code;

			for (int pass = 0; pass < 100; ++pass)
			{
				auto it = current_code.begin();
				auto backup = it;

				auto accept = [](uint64_t ic, std::list<uint64_t>::iterator& it)
				{
					if (*it == ic)
					{
						++it;
						return true;
					}
					return false;
				};
				while (it != current_code.end())
				{
					if (accept(ic_push_rax, it))
					{
						if (accept(ic_pop_rax, it))
						{
							continue;
						}
						optimized_code.push_back(ic_push_rax);
					}
					else if (accept(ic_mov_xmm_rax, it))
					{
						uint64_t a = *it;
						uint64_t b;
						++it;
						bool append_rax_xmm = false;
						if (accept(ic_mov_rax_xmm, it))
						{
							b = *it;
							++it;
							if (a == b)
							{
								continue;
							}
							else
							{
								append_rax_xmm = true;
							}
						}
						optimized_code.push_back(ic_mov_xmm_rax);
						optimized_code.push_back(a);
						if (append_rax_xmm)
						{
							optimized_code.push_back(ic_mov_rax_xmm);
							optimized_code.push_back(b);
						}
					}
					else
					{
						optimized_code.push_back(*it);
						++it;
					}
				}

				current_code = optimized_code;
				optimized_code.clear();
			}

			printf("Reduced by %d\n",(int)(m_intermidiate_code.size() - current_code.size()));
			m_intermidiate_code = current_code;
			//printf("After:\n");
			PrintIntermidiate();
			//printf("\n");
		}

		std::list<uint64_t> m_intermidiate_code;

		template<typename T>
		void Write(T val)
		{
			*reinterpret_cast<T*>(mem + pointer) = val;
			pointer += sizeof(T);
		}

		template<typename T>
		void write_v64b(T val)
		{
			uint64_t x = *reinterpret_cast<uint64_t* >(&val);
			m_intermidiate_code.push_back(ic_im_value);
			m_intermidiate_code.push_back(x);
		}

		template<typename T>
		void write_m64b(T val)
		{
			uint64_t x = *reinterpret_cast<uint64_t*>(&val);
			m_intermidiate_code.push_back(ic_address);
			m_intermidiate_code.push_back(x);
		}

		void movabs_rax()
		{
			m_intermidiate_code.push_back(ic_movabs_rax);
		}

		void mov_mrax_xmm(uint8_t x)
		{
			m_intermidiate_code.push_back(ic_mov_mrax_xmm);
			m_intermidiate_code.push_back(x);
		}

		void mov_rax_xmm(uint8_t x)
		{
			m_intermidiate_code.push_back(ic_mov_rax_xmm);
			m_intermidiate_code.push_back(x);
		}

		void mov_xmm_rax(uint8_t x)
		{
			m_intermidiate_code.push_back(ic_mov_xmm_rax);
			m_intermidiate_code.push_back(x);
		}

		void add_xmm1_xmm0()
		{
			m_intermidiate_code.push_back(ic_add_xmm1_xmm0);
		}

		void sub_xmm1_xmm0()
		{
			m_intermidiate_code.push_back(ic_sub_xmm1_xmm0);
		}

		void mul_xmm1_xmm0()
		{
			m_intermidiate_code.push_back(ic_mul_xmm1_xmm0);
		}

		void div_xmm1_xmm0()
		{
			m_intermidiate_code.push_back(ic_div_xmm1_xmm0);
		}

		void pxor_xmm1_xmm0()
		{
			m_intermidiate_code.push_back(ic_pxor_xmm1_xmm0);
		}

		void neg_xmm0()
		{
			movabs_rax();
			write_v64b(0x8000000000000000ULL);
			mov_rax_xmm(1);
			pxor_xmm1_xmm0();
		}

		void push_rax()
		{
			m_intermidiate_code.push_back(ic_push_rax);
		}

		void pop_rax()
		{
			m_intermidiate_code.push_back(ic_pop_rax);
		}

		void push_mrax()
		{
			m_intermidiate_code.push_back(ic_push_mrax);

		}

		void push_xmm(uint8_t x)
		{
			mov_xmm_rax(x);
			push_rax();
		}

		void pop_xmm(uint8_t x)
		{
			pop_rax();
			mov_rax_xmm(x);
		}

		void call_rax()
		{
			m_intermidiate_code.push_back(ic_call_rax);
		}

		void ret()
		{
			m_intermidiate_code.push_back(ic_ret);
		}

		uint16_t Symbol(const std::string& name)
		{
			auto it = symbols.find(name);
			if (it == symbols.end())
			{
				symbols[name] = idc;
				relocations_sym.push_back(idc);
				return idc++;
			}
			else
			{
				relocations_sym.push_back(it->second);
				return it->second;
			}
		}


		double (*Eval)();

		int pointer;
		uint16_t idc;
		int size;
		uint8_t* mem;

		std::map<std::string, uint16_t> symbols;
		std::vector<uint16_t> relocations_sym;
		std::vector<uint16_t> relocations_pointer;
	};
}
