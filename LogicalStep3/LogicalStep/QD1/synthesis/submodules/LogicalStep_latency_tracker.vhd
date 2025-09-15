library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity LogicalStep_latency_tracker is
port(	response		: in std_logic;
		pulse			: in std_logic;
		clk 			: in std_logic;
		reset			: in std_logic;
		enable		: in std_logic;
 	   missed_resp	: out STD_LOGIC;
		multi_resp	: out STD_LOGIC;
		missed		: out std_logic_vector(15 downto 0) := (others => '0');
		multi			: out std_logic_vector(15 downto 0) := (others => '0');
		latency 		: out std_logic_vector(15 downto 0) := (others => '0')
	);
end LogicalStep_latency_tracker;


	ARCHITECTURE a OF LogicalStep_latency_tracker IS
   TYPE STATE_TYPE IS (off, wait4cycle, wait4cycle_with_R, wait4cycle2_with_R, S1RX, S1R1, S1R0, S1R0_with_R, S1R02_with_R, S0R0, S0R1, missed_pulse, update, update2);
	
   SIGNAL state   											: STATE_TYPE;
	signal misses 												: integer := 0;
	signal multi_r												: integer := 0;
	signal currentLatency 									: integer := 0;
	
BEGIN


   PROCESS (clk)
   BEGIN
		IF (clk'EVENT AND clk = '1') THEN
			IF enable = '0' then 
				state <= off;
			else			
				CASE state IS
					WHEN off=>
						state <= wait4cycle;
						
					WHEN wait4cycle =>			-- wait for S to be asserted.
						IF pulse = '1' THEN
							state <= S1RX;
						ELSIF response = '1' THEN
							state <= wait4cycle_with_R;
						ELSE
							state <= wait4cycle;
						END IF;
						
					WHEN wait4cycle_with_R =>	-- response arrived while waiting for next cycle (multi response error). Active for 1 clock.
							state <= wait4cycle2_with_R;
						
					WHEN wait4cycle2_with_R =>	-- wait here until Stimulus pulse arrives
						IF pulse = '1' THEN
							state <= S1RX;
						ELSE
							state <= wait4cycle2_with_R;
						END IF;
						
					WHEN S1RX =>					-- S asserted but R not asserted yet
						IF response = '1' THEN
							state <= S1R1;
						ELSIF pulse = '1' THEN
							state <= S1RX;
						ELSE
							state <= S0R0;
						END IF;
						
					WHEN S1R1 =>					-- S asserted and R asserted
						IF response = '0' THEN
							state <= S1R0;
						ELSIF pulse = '0' THEN
							state <= S0R1;
						ELSE
							state <= S1R1;
						END IF;

					WHEN S1R0 =>					-- S asserted and  R asserted and de-asserted
						IF pulse = '0' THEN
							state <= update;
						ELSIF response = '1' THEN
							state <= S1R0_with_R;
						ELSE
							state <= S1R0;
						END IF;

					WHEN S1R0_with_R =>			-- S asserted and  a second R arrives. Active only for ONE Clock.
							state <= S1R02_with_R;

					WHEN S1R02_with_R =>			-- wait here until pulse is de-asserted.
						IF pulse = '0' THEN
							state <= update;
						ELSE
							state <= S1R02_with_R;
						END IF;

						
					WHEN S0R0 =>					-- S asserted and de-asserted but R not asserted yet
						IF response = '1' THEN
							state <= S0R1;
						ELSIF pulse = '1' THEN
							state <= missed_pulse;
						ELSE
							state <= S0R0;
						END IF;

					WHEN S0R1 =>					-- S asserted and de-asserted and R is asserted
						IF response = '0' THEN
							state <= update;
						ELSIF pulse = '1' THEN
							state <= missed_pulse;
						ELSE
							state <= S0R1;
						END IF;
						
					WHEN missed_pulse =>			-- active for 1 clock cycle
							state <= update;
													
					WHEN update =>	-- cycle is done; update the latency valueS
							state <= update2;

					WHEN update2 =>	-- update is done; prepare for next cycle
							state <= wait4cycle;
												
				END CASE;
			END IF;
      END IF;
   END PROCESS;
	
	PROCESS (clk)
		variable count           	: integer := 0;
		
   BEGIN
		if rising_edge(clk) then 
			CASE state IS
				WHEN off =>
					if enable = '1' then
					   missed_resp <= '0';
						multi_resp <= '0';
						misses <= 0;
						multi_r <= 0;
						currentLatency <= 0;
					end if;
					
				WHEN wait4cycle =>
					count := 0;
					missed_resp <= '0';
					multi_resp <= '0';
					
				WHEN S1RX =>
					count := count + 1;
					missed_resp <= '0';
					multi_resp <= '0';
					
				WHEN S0R0 =>
					count := count + 1;
					missed_resp <= '0';
					multi_resp <= '0';
										
				WHEN missed_pulse =>
					misses <= misses + 1;
					missed_resp <= '1';
					multi_resp <= '0';

				WHEN wait4cycle_with_R =>
					multi_r <= multi_r + 1;
					multi_resp <= '1';
					missed_resp <= '0';

				WHEN S1R0_with_R =>
					multi_r <= multi_r + 1;
					multi_resp <= '1';
					missed_resp <= '0';
					
				WHEN update =>
					if currentLatency = 0 then
						currentLatency <= count;
					else
						currentLatency <= currentLatency + ((count - currentLatency)/ 8); -- compute running average latency for current cycle
					end if;
					missed_resp <= '0';
					multi_resp <= '0';
	
				WHEN OTHERS =>
					missed_resp <= '0';
					multi_resp <= '0';
				
			END CASE;
		end if;

   END PROCESS;
	
	missed 		<= std_logic_vector(to_unsigned(misses, 16));
	multi		 	<= std_logic_vector(to_unsigned(multi_r, 16));
	latency 		<= std_logic_vector(to_unsigned(currentLatency, 16)); 
	

end a;