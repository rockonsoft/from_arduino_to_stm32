## HAL TIM callback explained
Question: Do I need to register the callback function `HAL_TIM_PeriodElapsedCallback()` in my code?
No — `HAL_TIM_PeriodElapsedCallback()` is a **predefined HAL callback function**, but you generally **do not register it yourself**.

The important distinction is between the **interrupt handler** and the **HAL callback**.

### The flow looks like this

For `TIM2`, when the timer reaches its period:

```text
TIM2 counter reaches ARR
        ↓
TIM2 update event
        ↓
TIM2 interrupt generated
        ↓
TIM2_IRQHandler()
        ↓
HAL_TIM_IRQHandler(&htim2)
        ↓
HAL_TIM_PeriodElapsedCallback(&htim2)
```

CubeMX generates the interrupt handler, typically in `stm32xxxx_it.c`:

```c
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}
```

The HAL's `HAL_TIM_IRQHandler()` then determines what happened and, for a period elapsed event, calls:

```c
HAL_TIM_PeriodElapsedCallback(&htim2);
```

### So where do you put your code?

You **implement/override** the callback in your application code.

For example:

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}
```

You don't normally put this in `stm32xxxx_it.c`. Put it in something like `main.c` or your own application source file.

### But where is the function defined?

The HAL provides a **weak default implementation**, conceptually something like:

```c
__weak void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(htim);
}
```

The `__weak` attribute is the key.

It allows you to provide your own implementation:

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Your implementation
}
```

The linker uses your implementation instead of the weak default one.

So there is **no callback registration** like:

```c
register_callback(HAL_TIM_PeriodElapsedCallback);
```

You simply implement the function with the correct signature.

---

### Starting the interrupt is a separate step

You also need to actually start TIM2 in interrupt mode:

```c
HAL_TIM_Base_Start_IT(&htim2);
```

For example:

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM2_Init();

    HAL_TIM_Base_Start_IT(&htim2);

    while (1)
    {
    }
}
```

Then your callback:

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}
```

The critical point is:

**`HAL_TIM_Base_Start_IT()` enables the timer's interrupt-driven operation.**

The callback itself is already part of the HAL's interrupt-processing mechanism.

This is also why your earlier CubeMX configuration matters: enabling **TIM2 global interrupt** causes CubeMX to generate the `TIM2_IRQHandler()` infrastructure, while `HAL_TIM_Base_Start_IT()` actually starts the timer in interrupt mode.
