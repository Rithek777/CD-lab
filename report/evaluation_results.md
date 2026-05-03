# Evaluation Results

- Compiler: `gfortran`
- Runs per executable: 3
- Platform: Windows-11-10.0.26200-SP0

## benchmark_array_temp

- Source: `examples\benchmark_array_temp.f90`
- Transformed source: `build\evaluation\transformed_benchmark_array_temp.f90`
- Transform status: ok

| Version | Compile | Run Status | Mean Seconds |
| --- | --- | --- | ---: |
| original | ok | ok | 0.058115 |
| transformed | ok | ok | 0.070791 |

### Allocation Tools

- original:
  - usr_bin_time: unavailable (/usr/bin/time is unavailable)
  - valgrind_massif: unavailable (valgrind is unavailable)
  - ld_preload_malloc_counter: unavailable (LD_PRELOAD malloc counting is not available on Windows)
- transformed:
  - usr_bin_time: unavailable (/usr/bin/time is unavailable)
  - valgrind_massif: unavailable (valgrind is unavailable)
  - ld_preload_malloc_counter: unavailable (LD_PRELOAD malloc counting is not available on Windows)

## benchmark_allocatable_realloc

- Source: `examples\benchmark_allocatable_realloc.f90`
- Transformed source: `build\evaluation\transformed_benchmark_allocatable_realloc.f90`
- Transform status: ok
- Transform messages: `line 12: skipped 'n = 1000 + iter': left-hand side looks like a scalar control variable
line 15: skipped 'checksum = checksum + sum(a)': left-hand side looks like a scalar control variable`

| Version | Compile | Run Status | Mean Seconds |
| --- | --- | --- | ---: |
| original | ok | ok | 0.056035 |
| transformed | ok | ok | 0.056578 |

### Allocation Tools

- original:
  - usr_bin_time: unavailable (/usr/bin/time is unavailable)
  - valgrind_massif: unavailable (valgrind is unavailable)
  - ld_preload_malloc_counter: unavailable (LD_PRELOAD malloc counting is not available on Windows)
- transformed:
  - usr_bin_time: unavailable (/usr/bin/time is unavailable)
  - valgrind_massif: unavailable (valgrind is unavailable)
  - ld_preload_malloc_counter: unavailable (LD_PRELOAD malloc counting is not available on Windows)

## benchmark_function_result

- Source: `examples\benchmark_function_result.f90`
- Transformed source: `build\evaluation\transformed_benchmark_function_result.f90`
- Transform status: ok
- Transform messages: `line 14: skipped 'checksum = checksum + sum(a)': left-hand side looks like a scalar control variable
line 27: skipped 'values(i) = real(i) * 0.5': not a simple array assignment`

| Version | Compile | Run Status | Mean Seconds |
| --- | --- | --- | ---: |
| original | ok | ok | 0.080379 |
| transformed | ok | ok | 0.046625 |

### Allocation Tools

- original:
  - usr_bin_time: unavailable (/usr/bin/time is unavailable)
  - valgrind_massif: unavailable (valgrind is unavailable)
  - ld_preload_malloc_counter: unavailable (LD_PRELOAD malloc counting is not available on Windows)
- transformed:
  - usr_bin_time: unavailable (/usr/bin/time is unavailable)
  - valgrind_massif: unavailable (valgrind is unavailable)
  - ld_preload_malloc_counter: unavailable (LD_PRELOAD malloc counting is not available on Windows)
