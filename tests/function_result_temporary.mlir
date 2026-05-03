module {
  func.func @function_result_temporary(%n: index) {
    %result = fir.allocmem !fir.array<?xf64> {fap.reason = "function result"} loc("examples/array_valued_function_result.f90":7:12)
    %call = fir.call @make_values(%n) : (index) -> !fir.heap<!fir.array<?xf64>>
    fir.freemem %result : !fir.heap<!fir.array<?xf64>>
    return
  }

  func.func private @make_values(index) -> !fir.heap<!fir.array<?xf64>>
}

