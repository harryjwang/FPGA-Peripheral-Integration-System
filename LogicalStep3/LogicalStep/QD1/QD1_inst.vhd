	component QD1 is
		port (
			audio_i2c_SDAT             : inout std_logic                     := 'X';             -- SDAT
			audio_i2c_SCLK             : out   std_logic;                                        -- SCLK
			audio_mclk_clk             : out   std_logic;                                        -- clk
			audio_out_ADCDAT           : in    std_logic                     := 'X';             -- ADCDAT
			audio_out_ADCLRCK          : in    std_logic                     := 'X';             -- ADCLRCK
			audio_out_BCLK             : in    std_logic                     := 'X';             -- BCLK
			audio_out_DACDAT           : out   std_logic;                                        -- DACDAT
			audio_out_DACLRCK          : in    std_logic                     := 'X';             -- DACLRCK
			button_pio_export          : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- export
			clk_50_clk                 : in    std_logic                     := 'X';             -- clk
			egm_interface_stimulus     : out   std_logic;                                        -- stimulus
			egm_interface_response     : in    std_logic                     := 'X';             -- response
			egm_interface_egm_leds     : out   std_logic_vector(3 downto 0);                     -- egm_leds
			lcd_display_RS             : out   std_logic;                                        -- RS
			lcd_display_RW             : out   std_logic;                                        -- RW
			lcd_display_data           : inout std_logic_vector(7 downto 0)  := (others => 'X'); -- data
			lcd_display_E              : out   std_logic;                                        -- E
			led_pio_export             : out   std_logic_vector(3 downto 0);                     -- export
			reset_n_reset_n            : in    std_logic                     := 'X';             -- reset_n
			response_out_export        : out   std_logic;                                        -- export
			sdram_0_addr               : out   std_logic_vector(11 downto 0);                    -- addr
			sdram_0_ba                 : out   std_logic_vector(1 downto 0);                     -- ba
			sdram_0_cas_n              : out   std_logic;                                        -- cas_n
			sdram_0_cke                : out   std_logic;                                        -- cke
			sdram_0_cs_n               : out   std_logic;                                        -- cs_n
			sdram_0_dq                 : inout std_logic_vector(15 downto 0) := (others => 'X'); -- dq
			sdram_0_dqm                : out   std_logic_vector(1 downto 0);                     -- dqm
			sdram_0_ras_n              : out   std_logic;                                        -- ras_n
			sdram_0_we_n               : out   std_logic;                                        -- we_n
			sdram_clk_clk              : out   std_logic;                                        -- clk
			segment_drive_segment_data : out   std_logic_vector(7 downto 0);                     -- segment_data
			segment_drive_digit1       : out   std_logic;                                        -- digit1
			segment_drive_digit2       : out   std_logic;                                        -- digit2
			stimulus_in_export         : in    std_logic                     := 'X';             -- export
			switch_pio_export          : in    std_logic_vector(7 downto 0)  := (others => 'X'); -- export
			uart_rxd                   : in    std_logic                     := 'X';             -- rxd
			uart_txd                   : out   std_logic;                                        -- txd
			spi_master_spi_master_cs   : out   std_logic;                                        -- spi_master_cs
			spi_master_spi_master_sclk : out   std_logic;                                        -- spi_master_sclk
			spi_master_spi_master_mosi : out   std_logic;                                        -- spi_master_mosi
			spi_master_spi_master_miso : in    std_logic                     := 'X';             -- spi_master_miso
			spi_master_spi_master_cd   : in    std_logic                     := 'X';             -- spi_master_cd
			spi_master_spi_master_wp   : in    std_logic                     := 'X'              -- spi_master_wp
		);
	end component QD1;

	u0 : component QD1
		port map (
			audio_i2c_SDAT             => CONNECTED_TO_audio_i2c_SDAT,             --     audio_i2c.SDAT
			audio_i2c_SCLK             => CONNECTED_TO_audio_i2c_SCLK,             --              .SCLK
			audio_mclk_clk             => CONNECTED_TO_audio_mclk_clk,             --    audio_mclk.clk
			audio_out_ADCDAT           => CONNECTED_TO_audio_out_ADCDAT,           --     audio_out.ADCDAT
			audio_out_ADCLRCK          => CONNECTED_TO_audio_out_ADCLRCK,          --              .ADCLRCK
			audio_out_BCLK             => CONNECTED_TO_audio_out_BCLK,             --              .BCLK
			audio_out_DACDAT           => CONNECTED_TO_audio_out_DACDAT,           --              .DACDAT
			audio_out_DACLRCK          => CONNECTED_TO_audio_out_DACLRCK,          --              .DACLRCK
			button_pio_export          => CONNECTED_TO_button_pio_export,          --    button_pio.export
			clk_50_clk                 => CONNECTED_TO_clk_50_clk,                 --        clk_50.clk
			egm_interface_stimulus     => CONNECTED_TO_egm_interface_stimulus,     -- egm_interface.stimulus
			egm_interface_response     => CONNECTED_TO_egm_interface_response,     --              .response
			egm_interface_egm_leds     => CONNECTED_TO_egm_interface_egm_leds,     --              .egm_leds
			lcd_display_RS             => CONNECTED_TO_lcd_display_RS,             --   lcd_display.RS
			lcd_display_RW             => CONNECTED_TO_lcd_display_RW,             --              .RW
			lcd_display_data           => CONNECTED_TO_lcd_display_data,           --              .data
			lcd_display_E              => CONNECTED_TO_lcd_display_E,              --              .E
			led_pio_export             => CONNECTED_TO_led_pio_export,             --       led_pio.export
			reset_n_reset_n            => CONNECTED_TO_reset_n_reset_n,            --       reset_n.reset_n
			response_out_export        => CONNECTED_TO_response_out_export,        --  response_out.export
			sdram_0_addr               => CONNECTED_TO_sdram_0_addr,               --       sdram_0.addr
			sdram_0_ba                 => CONNECTED_TO_sdram_0_ba,                 --              .ba
			sdram_0_cas_n              => CONNECTED_TO_sdram_0_cas_n,              --              .cas_n
			sdram_0_cke                => CONNECTED_TO_sdram_0_cke,                --              .cke
			sdram_0_cs_n               => CONNECTED_TO_sdram_0_cs_n,               --              .cs_n
			sdram_0_dq                 => CONNECTED_TO_sdram_0_dq,                 --              .dq
			sdram_0_dqm                => CONNECTED_TO_sdram_0_dqm,                --              .dqm
			sdram_0_ras_n              => CONNECTED_TO_sdram_0_ras_n,              --              .ras_n
			sdram_0_we_n               => CONNECTED_TO_sdram_0_we_n,               --              .we_n
			sdram_clk_clk              => CONNECTED_TO_sdram_clk_clk,              --     sdram_clk.clk
			segment_drive_segment_data => CONNECTED_TO_segment_drive_segment_data, -- segment_drive.segment_data
			segment_drive_digit1       => CONNECTED_TO_segment_drive_digit1,       --              .digit1
			segment_drive_digit2       => CONNECTED_TO_segment_drive_digit2,       --              .digit2
			stimulus_in_export         => CONNECTED_TO_stimulus_in_export,         --   stimulus_in.export
			switch_pio_export          => CONNECTED_TO_switch_pio_export,          --    switch_pio.export
			uart_rxd                   => CONNECTED_TO_uart_rxd,                   --          uart.rxd
			uart_txd                   => CONNECTED_TO_uart_txd,                   --              .txd
			spi_master_spi_master_cs   => CONNECTED_TO_spi_master_spi_master_cs,   --    spi_master.spi_master_cs
			spi_master_spi_master_sclk => CONNECTED_TO_spi_master_spi_master_sclk, --              .spi_master_sclk
			spi_master_spi_master_mosi => CONNECTED_TO_spi_master_spi_master_mosi, --              .spi_master_mosi
			spi_master_spi_master_miso => CONNECTED_TO_spi_master_spi_master_miso, --              .spi_master_miso
			spi_master_spi_master_cd   => CONNECTED_TO_spi_master_spi_master_cd,   --              .spi_master_cd
			spi_master_spi_master_wp   => CONNECTED_TO_spi_master_spi_master_wp    --              .spi_master_wp
		);

